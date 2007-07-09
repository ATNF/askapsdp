///
/// @file : Synthesis imaging program
///
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
void sendNE(MPIConnectionSet::ShPtr& cs, int rank, const NormalEquations& ne)
{
  LOFAR::BlobString bs;
  bs.resize(0);
  LOFAR::BlobOBufString bob(bs);
  LOFAR::BlobOStream out(bob);
  out.putStart("ne", 1);
  out << rank << ne;
  out.putEnd();
  cs->write(0, bs);
}


// Receive the normal equations as a blob
void receiveNE(MPIConnectionSet::ShPtr& cs, int nnode, Solver::ShPtr& is)
{
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
  return;
}


// Send the model to all workers
void sendModel(MPIConnectionSet::ShPtr& cs, int nnode, const Params& skymodel)
{
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
}


// Receive the model from the solver
void receiveModel(MPIConnectionSet::ShPtr& cs, Params& skymodel)
{
  LOFAR::BlobString bs;
  bs.resize(0);
  cs->read(0, bs);
  LOFAR::BlobIBufString bib(bs);
  LOFAR::BlobIStream in(bib);
  int version=in.getStart("model");
  CONRADASSERT(version==1)
  in >> skymodel;
  in.getEnd();
  return;
}


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
    bool isMaster(isParallel&&(rank==0));

    initOutput(rank);

    if(isParallel)
    {
      cs=initConnections(nnode, rank);
      if (isMaster)
      {
        MWCOUT << "CONRAD synthesis imaging program (parallel version) on " << nnode
          << " nodes (master)" << std::endl;
      }
      else
      {
        MWCOUT << "CONRAD synthesis imaging program (parallel version) on " << nnode
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

    Params skymodel;
/// Create the specified images from the definition in the
/// parameter set
    SynthesisParamsHelper::add(skymodel, parset, "Images.");

/// Create the gridder and solver using factories acting on
/// parametersets
    Solver::ShPtr solver=ImageSolverFactory::make(skymodel, subset);
    IVisGridder::ShPtr gridder=VisGridderFactory::make(subset);

    NormalEquations ne(skymodel);

// Now do the required number of major cycles
    int nCycles(parset.getInt32("Cimager.solver.cycles", 10));
    for (int cycle=0;cycle<nCycles;cycle++)
    {

      if(nCycles>1)
      {
        MWCOUT << "*** Starting major cycle " << cycle << " ***" << std::endl;
      }

/// Now iterate through all data sets
      int slot=1;
      vector<string> ms=parset.getStringVector("DataSet");
      for (vector<string>::iterator thisms=ms.begin();thisms!=ms.end();++thisms)
      {
        if(!isParallel||(rank==slot))
        {
          MWCOUT << "Processing data set " << *thisms << std::endl;
          TableDataSource ds(*thisms);
          IDataSelectorPtr sel=ds.createSelector();
          IDataConverterPtr conv=ds.createConverter();
          conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"Hz");
          IDataSharedIter it=ds.createIterator(sel, conv);
          it.init();
          it.chooseOriginal();
          if((cycle>0)&&(isParallel))
          {
            receiveModel(cs, skymodel);
            MWCOUT << "Received model from master" << std::endl;
          }
          ImageFFTEquation ie(skymodel, it, gridder);
          MWCOUT << "Constructed measurement equation" << std::endl;

          ie.calcEquations(ne);
          MWCOUT << "Calculated normal equations" << std::endl;
          if(nnode>1)
          {
            sendNE(cs, rank, ne);
            MWCOUT << "Sent normal equations to the solver via MPI" << std::endl;
          }
          else
          {
            solver->addNormalEquations(ne);
            MWCOUT << "Added normal equations to solver" << std::endl;
          }
        }
        slot++;
      }

// Solver runs in master
      if(!isParallel||isMaster)
      {
// Could be that we are waiting for normal equations
        if (isParallel)
        {
          MWCOUT << "Waiting for normal equations" << std::endl;
          receiveNE(cs, nnode, solver);
          MWCOUT << "Received all normal equations" << std::endl;
        }
// Perform the solution
        MWCOUT << "Solving normal equations" << std::endl;
        Quality q;
        solver->solveNormalEquations(q);
        MWCOUT << "Solved normal equations" << std::endl;
        skymodel=solver->parameters();
        if((nCycles>1)&&(isParallel))
        {
          sendModel(cs, nnode, skymodel);
        }
        MWCOUT << "Broadcast model to all workers" << std::endl;
        vector<string> resultimages=skymodel.names();
        for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
        {
          casa::Array<double> resultImage(skymodel.value(*it));
          MWCOUT << *it << std::endl
            << "Maximum = " << max(resultImage) << ", minimum = " << min(resultImage) << std::endl;
        }

      }
    }

// The solution is complete - now we need to write out the results
    if(!isParallel||isMaster)
    {
// Now write the results to a table
      string resultfile(parset.getString("Parms.Result"));
      ParamsCasaTable results(resultfile, false);
      results.setParameters(skymodel);

// And write the images to CASA image files
      vector<string> resultimages=skymodel.names();
      for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
      {
        SynthesisParamsHelper::saveAsCasaImage(skymodel, *it, *it);
      }
    }
    MWCOUT << "Finished imaging" << std::endl;
    MWCOUT << "user:   " << timer.user () << std::endl;
    MWCOUT << "system: " << timer.system () << std::endl;
    MWCOUT << "real:   " << timer.real () << std::endl;
    MWCOUT << "Ending MPI for rank " << rank << std::endl;
    MPIConnection::endMPI();

    exit(0);
  }
  catch (conrad::ConradError& x)
  {
    MWCOUT << "Conrad error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  catch (std::exception& x)
  {
    MWCOUT << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
};
