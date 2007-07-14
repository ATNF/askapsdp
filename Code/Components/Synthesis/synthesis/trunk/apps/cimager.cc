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

std::ostream& os() {
  if(MPIConnection::getRank()>0) {
    return MWCOUT;
  }
  else {
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

    Params skymodel;
    
/// Create the specified images from the definition in the
/// parameter set
    SynthesisParamsHelper::add(skymodel, parset, "Images.");
        
/// Create the gridder and solver using factories acting on
/// parametersets
    IVisGridder::ShPtr gridder=VisGridderFactory::make(subset);

    NormalEquations ne(skymodel);

// Now do the required number of major cycles
    int nCycles(parset.getInt32("Cimager.solver.cycles", 1));
    for (int cycle=0;cycle<nCycles;cycle++)
    {

      Solver::ShPtr solver=ImageSolverFactory::make(skymodel, subset);

      if(nCycles>1)
      {
        os() << "*** Starting major cycle " << cycle << " ***" << std::endl;
      }

/// Now iterate through all data sets
/// If running in parallel, the master doesn't do this.
      if(!isParallel||!isMaster) {
      int slot=1;
      vector<string> ms=parset.getStringVector("DataSet");
      for (vector<string>::iterator thisms=ms.begin();thisms!=ms.end();++thisms)
      {
        if(!isParallel||(rank==slot))
        {
            os() << "Processing data set " << *thisms << std::endl;
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
              os() << "Received model from master" << std::endl;
            }
            ImageFFTEquation ie(skymodel, it, gridder);
            os() << "Constructed measurement equation" << std::endl;
  
            ie.calcEquations(ne);
            os() << "Calculated normal equations" << std::endl;
            if(isParallel)
            {
              sendNE(cs, rank, ne);
              os() << "Sent normal equations to the solver via MPI" << std::endl;
            }
            else
            {
              solver->addNormalEquations(ne);
              os() << "Added normal equations to solver" << std::endl;
            }
          }
          slot++;
          os() << "user:   " << timer.user () << " system: " << timer.system () 
            <<" real:   " << timer.real () << std::endl;      }
      }

// Now do the solution
// If running in parallel only the master does this
      if(!isParallel||isMaster)
      {
// Could be that we are waiting for normal equations
        if (isParallel)
        {
          os() << "Waiting for normal equations" << std::endl;
          receiveNE(cs, nnode, solver);
          os() << "Received all normal equations" << std::endl;
        }
        if(cycle<(nCycles-1)) {
          os() << "Solving normal equations" << std::endl;
          Quality q;
          solver->solveNormalEquations(q);
          os() << "Solved normal equations" << std::endl;
          skymodel=solver->parameters();
          // Don't send the model where there are no workers around
          if((nCycles>1)&&(isParallel))
          {
            sendModel(cs, nnode, skymodel);
            os() << "Sent model to all workers" << std::endl;
          }
        }
        else {
          os() << "Writing out result as an image" << std::endl;
          vector<string> resultimages=skymodel.names();
          for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
          {
            SynthesisParamsHelper::saveAsCasaImage(skymodel, *it, *it);
          }
          if(parset.getBool("Cimager.restore", true)) {
            vector<string> beam=parset.getStringVector("Cimager.restore.beam");
            casa::Vector<casa::Quantum<double> > qbeam(3);
            for (int i=0;i<3;i++) {
              casa::Quantity::read(qbeam(i), beam[i]);
            }
            os() << "Last cycle - restoring model" << std::endl;
            // Make an image restore solver from the current solver
            // so it can use the normal equations
            // And write the images to CASA image files before and after restoring
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
        vector<string> resultimages=skymodel.names();
        for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
        {
          casa::Array<double> resultImage(skymodel.value(*it));
          os() << *it << std::endl
            << "Maximum = " << max(resultImage) << ", minimum = " 
            << min(resultImage) << std::endl;
        }

        os() << "user:   " << timer.user () << " system: " << timer.system () 
          <<" real:   " << timer.real () << std::endl;      }
    }

// The solution is complete - now we need to write out the results
    if(!isParallel||isMaster)
    {
// Now write the results to a table
      string resultfile(parset.getString("Parms.Result"));
      ParamsCasaTable results(resultfile, false);
      results.setParameters(skymodel);
    }
    os() << "Finished imaging" << std::endl;
    if(isParallel) {
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
