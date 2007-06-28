//#  tMWControl.cc: Test program for Master-Worker framework
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include "PredifferTest.h"
#include "SolverTest.h"
#include "MWIos.h"
#include <mwcontrol/ParameterHandlerBBS.h>
#include <mwcontrol/MWStrategySpec.h>
#include <mwcontrol/MWSpec2Step.h>
#include <mwcommon/MemConnectionSet.h>
#include <mwcommon/SocketConnectionSet.h>
#include <mwcommon/MPIConnectionSet.h>
#include <mwcommon/WorkerFactory.h>
#include <mwcommon/WorkerControl.h>
#include <mwcommon/MasterControl.h>
#include <mwcommon/MWMultiStep.h>
#include <mwcontrol/MWSolveStepBBS.h>
#include <mwcontrol/MWCorrectStepBBS.h>
#include <mwcontrol/MWSubtractStepBBS.h>
#include <mwcontrol/MWPredictStepBBS.h>
#include <mwcommon/VdsDesc.h>
#include <mwcommon/MWError.h>
#include <APS/ParameterSet.h>
#include <iostream>

using namespace conrad::cp;
using namespace LOFAR;
using namespace std;


void setAllWorkers (MWConnectionSet::ShPtr& prediffers,
                    MWConnectionSet::ShPtr& solvers,
                    MWConnectionSet& workers, int nworkers,
                    const WorkerFactory& factory)
{
  // If there are no remote workers, just copy the workers to prediffers.
  if (nworkers == 0) {
    prediffers = workers.clone();
  } else {
    vector<int> predInx;
    vector<int> solvInx;
    predInx.reserve (nworkers);
    // We have to read from every worker and see what it can do.
    BlobString buf;
    for (int i=0; i<nworkers; ++i) {
      workers.read (i, buf);
      WorkerInfo info = WorkerProxy::getWorkerInfo (buf);
      if (info.getWorkType() == 0) {
        predInx.push_back(i);
      } else {
        solvInx.push_back(i);
      }
    }
    prediffers = workers.clone (predInx);
    solvers = workers.clone (solvInx);
  }
  // If there are no solvers, make a local solver.
  if (!solvers  ||  solvers->size() == 0) {
    MemConnectionSet* sv = new MemConnectionSet();
    solvers = MWConnectionSet::ShPtr(sv);
    sv->addConnection (factory.create("Solver"));
  }
}

void doMaster (const string& port,
               int solverRank,
               int nworkers, int nparts,
	       const WorkerFactory& factory,
	       const ParameterHandlerBBS& params)
{
  // Get the initial values from the params.
  string msName, colName, skyDB, instDB;
  unsigned subBand;
  bool calcUVW;
  params.getInitInfo (msName, colName, skyDB, instDB, subBand, calcUVW);
  // Get the full observation domain for this MS.
  VdsDesc msDesc(msName+".cfg");
  const VdsPartDesc& vdsDesc = msDesc.getDesc();
  ObsDomain fullDomain;
  fullDomain.setTime (vdsDesc.getStartTime(),
		      vdsDesc.getEndTime());
  fullDomain.setFreq (vdsDesc.getStartFreqs()[0],
		      vdsDesc.getEndFreqs()[vdsDesc.getNBand() - 1]);
  // Setup connections for the prediffers and solvers.
  MWConnectionSet::ShPtr workers;
  // Set up the connection for all workers.
  // Use socket connection if required, otherwise MPI connection if possible.
  // If MPI is impossible, use memory connection for a prediffer per VDS part.
  if (! port.empty()) {
    SocketConnectionSet* workConns (new SocketConnectionSet(port));
    workers = MWConnectionSet::ShPtr (workConns);
    workConns->addConnections (nworkers);
  } else if (nworkers > 0) {
    MPIConnectionSet* workConns (new MPIConnectionSet());
    workers = MWConnectionSet::ShPtr (workConns);
    for (int i=0; i<nworkers; ++i) {
      // A solver has MPI tag 1.
      int tag = i>=solverRank ? 0:1;
      workConns->addConnection (i+1, tag);
    }
  } else {
    MemConnectionSet* workConns (new MemConnectionSet());
    workers = MWConnectionSet::ShPtr (workConns);
    for (int i=0; i<nparts; ++i) {
      workConns->addConnection (factory.create("Prediffer"));
    }
  }
  // Find out what all remote workers can do.
  // They send a message with their capabilities after the connection is made.
  // So read from all workers and put in appropriate set.
  MWConnectionSet::ShPtr prediffers;
  MWConnectionSet::ShPtr solvers;
  setAllWorkers (prediffers, solvers, *workers, nworkers, factory);
  // Check if there are enough prediffers.
  if (prediffers->size() < nparts) {
    CONRADTHROW(MWError, "The Visibility Data Set is split into "
                << nparts << " parts, so mwcontrol has to have at least "
                << nparts+1 << " prediffers, but only " << prediffers->size()
                << " are available");
  }
  // Create the master control and initialize it.
  MasterControl mc (prediffers, solvers);
  mc.setInitInfo (msName, colName, skyDB, instDB, subBand, calcUVW,
		  fullDomain);
  // Assemble all steps defined in the parameters into a single spec.
  vector<MWStrategySpec> strategySpecs = params.getStrategies();
  // Loop through all strategies.
  for (vector<MWStrategySpec>::const_iterator iter=strategySpecs.begin();
       iter!=strategySpecs.end();
       ++iter) {
    mc.setWorkDomainSpec (MWSpec2Step::convertStrategy (*iter));
    // Convert the specifications into MWSteps.
    MWSpec2Step converter;
    iter->getStep().visit (converter);
    // Execute the steps.
    mc.processSteps (converter.getSteps());
  }
  mc.quit();
}

void doPrediffer (const string& host, const string& port,
                  const WorkerFactory& factory)
{
  MWCOUT << "prediffer rank " << MPIConnection::getRank() << endl;
  WorkerControl pc(factory.create ("Prediffer"));
  // Connect to master on rank 0.
  if (port.empty()) {
    pc.init (MWConnection::ShPtr(new MPIConnection(0, 0)));
  } else {
    pc.init (MWConnection::ShPtr(new SocketConnection(host, port)));
  }
  pc.run();
}

void doSolver (const string& host, const string& port,
               const WorkerFactory& factory)
{
  MWCOUT << "solver rank " << MPIConnection::getRank() << endl;
  WorkerControl sc(factory.create ("Solver"));
  // Connect to master on rank 0.
  if (port.empty()) {
    sc.init (MWConnection::ShPtr(new MPIConnection(0, 1)));
  } else {
    sc.init (MWConnection::ShPtr(new SocketConnection(host, port)));
  }
  sc.run();
}

void findSocket (int argc, const char** argv,
                 string& host, string& port, int& nnode, int& rank)
{
  CONRADCHECK (argc >= 6, "Using sockets run as: tMWControl socket <host> "
               "<port> <#nodes> <rank>");
  host = argv[2];
  port = argv[3];
  istringstream iss(argv[4]);
  iss >> nnode;
  istringstream iss1(argv[5]);
  iss1 >> rank;
}

int main (int argc, const char** argv)
{
  int status = 0;
  try {
    // Register the create functions for the various steps.
    MWSolveStepBBS::registerCreate();
    MWCorrectStepBBS::registerCreate();
    MWSubtractStepBBS::registerCreate();
    MWPredictStepBBS::registerCreate();
    MWMultiStep::registerCreate();
    // Define the functions to use for the proxy workers.
    WorkerFactory factory;
    factory.push_back ("Prediffer", PredifferTest::create);
    factory.push_back ("Solver", SolverTest::create);
    // Initialize MPI (also succeeds if no MPI available).
    MPIConnection::initMPI (argc, argv);
    int nnode = MPIConnection::getNrNodes();
    int rank  = MPIConnection::getRank();
    // If only one MPI node, we may run in a single process
    // or in multiple processes connected via sockets.
    // Find out from argv.
    string host, port;
    if (nnode == 1) {
      if (argc > 1  &&  string(argv[1]) == "socket") {
        findSocket (argc, argv, host, port, nnode, rank);
      }
    }
    {
      // Set the name of the output stream.
      std::ostringstream ostr;
      ostr << rank;
      MWIos::setName ("tMWControl_tmp.cout" + ostr.str());
    }
    // Open the parameter set and get nr of VDS parts.
    ParameterHandlerBBS params(LOFAR::ACC::APS::ParameterSet("tMWControl.in"));
    int nparts = params.getNParts();
    // Find out if this process is master, solver, or prediffer.
    int solverRank = 0;
    if (nnode > nparts+1) {
      // The master and solver can run on different nodes.
      solverRank = 1;
    }
    // Initialize and run the controls.
    if (rank == 0) {
      doMaster (port, solverRank, nnode-1, nparts, factory, params);
    } else if (rank > solverRank) {
      doPrediffer (host, port, factory);
    } else {
      doSolver (host, port, factory);
    }
    MPIConnection::endMPI();
  } catch (std::exception& x) {
    cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << endl;
    status = 1;
  }
  exit(status);
}
