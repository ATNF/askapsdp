///
/// @file : Synthesis imaging program
///
/// Performs synthesis imaging from a data source, using any of a number of
/// image solvers. Can run in serial or parallel (MPI) mode.
///
/// The data are accessed from the DataSource. This is and will probably remain
/// disk based. The images are kept purely in memory until the end.
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#include <conrad/ConradError.h>

#include <mwcommon/MPIConnection.h>
#include <mwcommon/MPIConnectionSet.h>
#include <mwcommon/MWIos.h>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>

#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/ImageSolverFactory.h>
#include <gridding/VisGridderFactory.h>

#include <fitting/ParamsCasaTable.h>
#include <fitting/Axes.h>

#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataSource.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/Timer.h>

#include <APS/ParameterSet.h>

#include <stdexcept>
#include <iostream>

using std::cout;
using std::endl;

using namespace conrad;
using namespace conrad::scimath;
using namespace conrad::synthesis;
using namespace LOFAR::ACC::APS;
using namespace conrad::cp;

// Support functions needed for parallel processing

std::ostream& os()
{
  if(MPIConnection::getRank()>0)
  {
    return MWCOUT;
  }
  else
  {
    return std::cout;
  }
}


// For MPI, we need to divert the output
void initOutput(int rank)
{
// Set the name of the output stream.
  std::ostringstream ostr;
  ostr << rank;
  MWIos::setName ("cimager_tmp.cout" + ostr.str());
}


// Initialize connections
MPIConnectionSet::ShPtr initConnections(int nnode, int rank)
{
  MPIConnectionSet* conns (new MPIConnectionSet());
  MPIConnectionSet::ShPtr cs(conns);
  if(rank==0)
  {
// I am the master - I need a connection to every worker
    for (int i=1; i<nnode; ++i)
    {
      cs->addConnection (i, 0);
    }
  }
  else
  {
// I am a worker - I only need to talk to the master
    cs->addConnection (0, 0);
  }
  return cs;
}


// Send the normal equations from this worker to the master as a blob
void sendNE(MPIConnectionSet::ShPtr& cs, int nnode, int rank, const NormalEquations& ne)
{
  if (nnode==1) return;
  casa::Timer timer;
  timer.mark();

  LOFAR::BlobString bs;
  bs.resize(0);
  LOFAR::BlobOBufString bob(bs);
  LOFAR::BlobOStream out(bob);
  out.putStart("ne", 1);
  out << rank << ne;
  out.putEnd();
  cs->write(0, bs);
  os() << "PREDIFFER Sent normal equations to the solver via MPI in "
    << timer.real() << " seconds " << std::endl;
}


// Receive the normal equations as a blob
void receiveNE(MPIConnectionSet::ShPtr& cs, int nnode, Solver::ShPtr& is)
{
  if (nnode==1) return;
  os() << "SOLVER Waiting for normal equations" << std::endl;
  casa::Timer timer;
  timer.mark();

  LOFAR::BlobString bs;
  NormalEquations ne;
  int rank;
  for (int i=0;i<nnode-1;i++)
  {
    cs->read(i, bs);
    LOFAR::BlobIBufString bib(bs);
    LOFAR::BlobIStream in(bib);
    int version=in.getStart("ne");
    CONRADASSERT(version==1);
    in >> rank >> ne;
    in.getEnd();
    is->addNormalEquations(ne);
  }
  os() << "SOLVER Received normal equations from the prediffers via MPI in "
    << timer.real() << " seconds" << std::endl;
  return;
}


// Send the model to all workers
void sendModel(MPIConnectionSet::ShPtr& cs, int nnode, const Params& skymodel)
{
  if (nnode==1) return;
  casa::Timer timer;
  timer.mark();

  LOFAR::BlobString bs;
  bs.resize(0);
  LOFAR::BlobOBufString bob(bs);
  LOFAR::BlobOStream out(bob);
  out.putStart("model", 1);
  out << skymodel;
  out.putEnd();
  for (int i=1; i<nnode; ++i)
  {
    cs->write(i-1, bs);
  }
  os() << "SOLVER Sent model to the prediffers via MPI in "
    << timer.real() << " seconds " << std::endl;
}


// Receive the model from the solver
void receiveModel(MPIConnectionSet::ShPtr& cs, int nnode, Params& skymodel)
{
  if (nnode==1) return;
  casa::Timer timer;
  timer.mark();
  LOFAR::BlobString bs;
  bs.resize(0);
  cs->read(0, bs);
  LOFAR::BlobIBufString bib(bs);
  LOFAR::BlobIStream in(bib);
  int version=in.getStart("model");
  CONRADASSERT(version==1);
  in >> skymodel;
  in.getEnd();
  os() << "PREDIFFER Received model from the solver via MPI in "
    << timer.real() << " seconds " << std::endl;
}


/// Calculate the normal equations for a given measurement set
void calcNE(const string& ms, Params& skymodel, IVisGridder::ShPtr& gridder,
NormalEquations& ne)
{
  os() << "PREDIFFER Calculating normal equations for " << ms << std::endl;
  casa::Timer timer;
  timer.mark();
  TableDataSource ds(ms);
  IDataSelectorPtr sel=ds.createSelector();
  IDataConverterPtr conv=ds.createConverter();
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"Hz");
  IDataSharedIter it=ds.createIterator(sel, conv);
  it.init();
  it.chooseOriginal();
  ImageFFTEquation ie(skymodel, it, gridder);
  ie.calcEquations(ne);
  os() << "PREDIFFER Calculated normal equations for " << ms << " in "
    << timer.real() << " seconds " << std::endl;
}


void solveNE(Params& skymodel, Solver::ShPtr& solver)
{
  os() << "SOLVER Solving normal equations" << std::endl;
  casa::Timer timer;
  timer.mark();
  Quality q;
  solver->solveNormalEquations(q);
  os() << "SOLVER Solved normal equations in "
    << timer.real() << " seconds " << std::endl;
  skymodel=solver->parameters();
}


void summariseModel(Params& skymodel)
{
  vector<string> resultimages=skymodel.names();
  for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
  {
    casa::Array<double> resultImage(skymodel.value(*it));
    os() << *it << std::endl
      << "Maximum = " << max(resultImage) << ", minimum = "
      << min(resultImage) << std::endl;
  }
}


/// Write the results out
void writeResults(Params& skymodel, Solver::ShPtr& solver,
const string& resultfile, bool restore,
const casa::Vector<casa::Quantum<double> > qbeam)
{
  vector<string> resultimages=skymodel.names();
  for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
  {
    SynthesisParamsHelper::saveAsCasaImage(skymodel, *it, *it);
  }
  if(resultfile!="")
  {
    ParamsCasaTable results(resultfile, false);
    results.setParameters(skymodel);
  }
  if(restore)
  {
/// Make an image restore solver from the current solver
/// so it can use the normal equations
/// And write the images to CASA image files before and after restoring
    vector<string> resultimages=skymodel.names();
    for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
    {
      SynthesisParamsHelper::saveAsCasaImage(skymodel, *it, *it);
    }
    ImageRestoreSolver ir(skymodel, qbeam);
    ir.copyNormalEquations(*solver);
    Quality q;
    ir.solveNormalEquations(q);
    for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
    {
      SynthesisParamsHelper::saveAsCasaImage(skymodel, *it, *it+string(".restored"));
    }
  }
}


void processInputs(const string& parsetname, string& resultfile, bool& restore,
int& nCycles, vector<string>& ms, casa::Vector<casa::Quantum<double> >& qbeam,
Params& skymodel, Solver::ShPtr& solver, IVisGridder::ShPtr& gridder)
{
/// Process inputs
  ParameterSet parset(parsetname);
  ParameterSet subset(parset.makeSubset("Cimager."));

  resultfile=parset.getString("Parms.Result", "");

  restore=parset.getBool("Cimager.restore", true);

  nCycles=parset.getInt32("Cimager.solver.cycles", 1);

  ms=parset.getStringVector("DataSet");

  qbeam.resize(3);
  vector<string> beam=parset.getStringVector("Cimager.restore.beam");
  for (int i=0;i<3;i++)
  {
    casa::Quantity::read(qbeam(i), beam[i]);
  }

/// Create the specified images from the definition in the
/// parameter set. We can solve for any number of images
/// at once (but you may/will run out of memory!)
  SynthesisParamsHelper::add(skymodel, parset, "Images.");

/// Create the solver from the parameterset definition and the existing
/// definition of the parameters. We create the solver here so that it
/// can do any caching required
  solver=ImageSolverFactory::make(skymodel, subset);

/// Create the gridder using a factory acting on a
/// parameterset
  gridder=VisGridderFactory::make(subset);
}


// Main function

int main(int argc, const char** argv)
{
  try
  {
///==============================================================================

// Initialize MPI (also succeeds if no MPI available).
    MPIConnection::initMPI (argc, argv);
    int nnode = MPIConnection::getNrNodes();
    int rank  = MPIConnection::getRank();
    MPIConnectionSet::ShPtr cs;

    bool isParallel(nnode>1);
    bool isSolver(rank==0);

// For MPI, send all output to separate log files. Eventually we'll do this
// using the log system.
    initOutput(rank);

///==============================================================================
/// Process inputs from the parset file
    string parsetname("cimager.in");

    string resultfile;                            // File for params
    bool restore;                                 // Do we want a restored image?
    int nCycles;                                  // Number of major cycles
    vector<string> ms;                            // Names of measurement sets
    casa::Vector<casa::Quantum<double> > qbeam;   // Restoring beam
    Params skymodel;                              // Skymodel
    Solver::ShPtr solver;                         // Image solver to be used
    IVisGridder::ShPtr gridder;                   // Gridder to be used

    processInputs(parsetname, resultfile, restore, nCycles, ms, qbeam,
      skymodel, solver, gridder);

    casa::Timer timer;
    timer.mark();


    NormalEquations ne(skymodel);
      
///==============================================================================
/// Parallel version
    if(isParallel)
    {
      cs=initConnections(nnode, rank);
      if (isSolver)
///
/// SOLVER steps
///
      {
        os() << "CONRAD synthesis imaging program (parallel version) on " << nnode
          << " nodes (master)" << std::endl;
        for (int cycle=0;cycle<nCycles;cycle++)
        {

          if(nCycles>1)
          {
            os() << "*** Starting major cycle " << cycle << " ***" << std::endl;
          }
/// We must be waiting for normal equations
          receiveNE(cs, nnode, solver);
/// Do a solution and send the model to the PREDIFFERs
          if(cycle<(nCycles-1))
          {
            solveNE(skymodel, solver);
            sendModel(cs, nnode, skymodel);
          }
          else
          {
  /// This is the final step - restore the image and write it out
            os() << "Writing out results as CASA images" << std::endl;
            writeResults(skymodel, solver, resultfile, restore, qbeam);
          }
          summariseModel(skymodel);
        }
      }
      else
      {
///
/// PREDIFFER steps
///
        os() << "CONRAD synthesis imaging program (parallel version) on " << nnode
          << " nodes (worker " << rank << ")" << std::endl;

/// Now iterate through all data sets in turn, accumulating the normal equations
/// via the PREDIFFERs. The normal equations are sent to the SOLVER
///
        for (int cycle=0;cycle<nCycles;cycle++)
        {
          if(cycle>0)
          {
            receiveModel(cs, nnode, skymodel);
          }
          calcNE(ms[rank-1], skymodel, gridder, ne);
          sendNE(cs, nnode, rank, ne);
          os() << "user:   " << timer.user () << " system: " << timer.system ()
            <<" real:   " << timer.real () << std::endl;
        }
      }
      os() << "Finished imaging" << std::endl;
      os() << "Ending MPI for rank " << rank << std::endl;
      MPIConnection::endMPI();
    }
    else
    {
///==============================================================================
/// Serial version
      os() << "CONRAD synthesis imaging program (serial version)" << endl;

      for (int cycle=0;cycle<nCycles;cycle++)
      {

        if(nCycles>1)
        {
          os() << "*** Starting major cycle " << cycle << " ***" << std::endl;
        }

/// Now iterate through all data sets in turn, accumulating the normal equations
/// via the PREDIFFERs. The normal equations are sent to the SOLVER
///
/// PREDIFFER step for all measurement sets
///
        for (vector<string>::iterator thisms=ms.begin();thisms!=ms.end();++thisms)
        {
          calcNE(*thisms, skymodel, gridder, ne);
          solver->addNormalEquations(ne);
          os() << "Added normal equations to solver " << std::endl;
        }
///
/// SOLVER does the solution
///
        if(cycle<(nCycles-1))
        {
          solveNE(skymodel, solver);
        }
        else
        {
/// This is the final step - restore the image and write it out
          os() << "Writing out results as CASA images" << std::endl;
          writeResults(skymodel, solver, resultfile, restore, qbeam);
        }
        summariseModel(skymodel);
        os() << "user:   " << timer.user () << " system: " << timer.system ()
          <<" real:   " << timer.real () << std::endl;
      }
      os() << "Finished imaging" << std::endl;
    }
///==============================================================================
    exit(0);
  }
  catch (conrad::ConradError& x)
  {
    std::cerr << "Conrad error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  catch (std::exception& x)
  {
    std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }


};
