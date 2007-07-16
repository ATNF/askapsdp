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
  os() << "Sent normal equations to the solver via MPI in " 
    << timer.real() << " seconds " << std::endl;
}


// Receive the normal equations as a blob
void receiveNE(MPIConnectionSet::ShPtr& cs, int nnode, Solver::ShPtr& is)
{
  if (nnode==1) return;
  os() << "Waiting for normal equations" << std::endl;
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
  os() << "Received normal equations from the workers via MPI in " 
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
  os() << "Sent model to the workers via MPI in " 
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
  os() << "Received model from the solver via MPI in " 
    << timer.real() << " seconds " << std::endl;
  return;
}

/// Calculate the normal equations for a given measurement set
void calcNE(const string& ms, Params& skymodel, IVisGridder::ShPtr& gridder,
  NormalEquations& ne) { 
  os() << "Calculating normal equations for " << ms << std::endl;
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
  os() << "Calculated normal equations for " << ms << " in " 
    << timer.real() << " seconds " << std::endl;
}

/// Write the results out
void writeResults(Params& skymodel, Solver::ShPtr& solver, ParameterSet& parset)
{
  vector<string> resultimages=skymodel.names();
  for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
  {
    SynthesisParamsHelper::saveAsCasaImage(skymodel, *it, *it);
  }
  string resultfile(parset.getString("Parms.Result", ""));
  if(resultfile!="") {
    ParamsCasaTable results(resultfile, false);
    results.setParameters(skymodel);
  }
  if(parset.getBool("Cimager.restore", true))
  {
    vector<string> beam=parset.getStringVector("Cimager.restore.beam");
    casa::Vector<casa::Quantum<double> > qbeam(3);
    for (int i=0;i<3;i++)
    {
      casa::Quantity::read(qbeam(i), beam[i]);
    }
    os() << "Last cycle - restoring model" << std::endl;
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

 

// Main function

int main(int argc, const char** argv)
{
  try
  {

// Initialize MPI (also succeeds if no MPI available).
    MPIConnection::initMPI (argc, argv);
    int nnode = MPIConnection::getNrNodes();
    int rank  = MPIConnection::getRank();
    MPIConnectionSet::ShPtr cs;

    bool isParallel(nnode>1);
    bool isMaster(rank==0);

    // For MPI, send all output to separate log files. Eventually we'll do this
    // using the log system.
    initOutput(rank);

    // Tell the world about me/us
    if(isParallel)
    {
      cs=initConnections(nnode, rank);
      if (isMaster)
      {
        os() << "CONRAD synthesis imaging program (parallel version) on " << nnode
          << " nodes (master)" << std::endl;
      }
      else
      {
        os() << "CONRAD synthesis imaging program (parallel version) on " << nnode
          << " nodes (worker " << rank << ")" << std::endl;
      }
    }
    else
    {
      cout << "CONRAD synthesis imaging program (serial version)" << endl;
    }

    casa::Timer timer;
    timer.mark();

    string parsetname("cimager.in");
    ParameterSet parset(parsetname);
    ParameterSet subset(parset.makeSubset("Cimager."));

// Define empty params to hold the images
    Params skymodel;

/// Create the specified images from the definition in the
/// parameter set. We can solve for any number of images
/// at once (but you may/will run out of memory!)
    SynthesisParamsHelper::add(skymodel, parset, "Images.");
    NormalEquations ne(skymodel);

/// Create the gridder using a factory acting on a
/// parameterset
    IVisGridder::ShPtr gridder=VisGridderFactory::make(subset);

///==============================================================================
// Now do the required number of major cycles
    int nCycles(parset.getInt32("Cimager.solver.cycles", 1));
    for (int cycle=0;cycle<nCycles;cycle++)
    {

/// Create the solver from the parameterset definition and the existing
/// definition of the parameters. We need the solver here so that we
/// can accumulate the normalequations
      Solver::ShPtr solver=ImageSolverFactory::make(skymodel, subset);

      if(nCycles>1)
      {
        os() << "*** Starting major cycle " << cycle << " ***" << std::endl;
      }

/// Now iterate through all data sets in turn, accumulating the normal equations.
/// The normal equations are sent to the solver, either in process (serial) or
/// via MPI (parallel)

/// If running in parallel, the master doesn't do this.
///
/// PREDIFFER steps
///
      vector<string> ms=parset.getStringVector("DataSet");
      if(isParallel) {
        if(!isMaster) {
          if(cycle>0) {
            receiveModel(cs, nnode, skymodel);
          } 
          calcNE(ms[rank-1], skymodel, gridder, ne);
          sendNE(cs, nnode, rank, ne);
        }
      }
      else {
        for (vector<string>::iterator thisms=ms.begin();thisms!=ms.end();++thisms)
        {
          calcNE(*thisms, skymodel, gridder, ne);
          solver->addNormalEquations(ne);
          os() << "Added normal equations to solver " << std::endl;
        }
      }

/// SOLVER steps
/// Now do the solution
/// If running in parallel only the master does this
      if(isMaster) 
      {
        if(isParallel) {
/// We must be waiting for normal equations
          receiveNE(cs, nnode, solver);
        }
        if(cycle<(nCycles-1))
        {
          os() << "Solving normal equations" << std::endl;
          Quality q;
          solver->solveNormalEquations(q);
          os() << "Solved normal equations" << std::endl;
          skymodel=solver->parameters();
          if(isParallel) {
            sendModel(cs, nnode, skymodel);
          }
        }
        else
        {
          /// This is the final step - restore the image and write it out
          os() << "Writing out results as CASA images" << std::endl;
          writeResults(skymodel, solver, parset);
        }
        vector<string> resultimages=skymodel.names();
        for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
        {
          casa::Array<double> resultImage(skymodel.value(*it));
          os() << *it << std::endl
            << "Maximum = " << max(resultImage) << ", minimum = "
            << min(resultImage) << std::endl;
        }
        os() << "user:   " << timer.user () << " system: " << timer.system ()
          <<" real:   " << timer.real () << std::endl;
      }
    } 
    /// End of major cycle
///==============================================================================

    os() << "Finished imaging" << std::endl;
    if(isParallel)
    {
      os() << "Ending MPI for rank " << rank << std::endl;
      MPIConnection::endMPI();
    }

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
