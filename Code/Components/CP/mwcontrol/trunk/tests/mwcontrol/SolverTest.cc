//#  SolverTest.cc: Solver BBSler of distributed VDS processing
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include "SolverTest.h"
#include "MWIos.h"
#include <mwcommon/MWSolveStep.h>
#include <mwcommon/MasterControl.h>
#include <mwcommon/MWBlobIO.h>
#include <mwcommon/MWError.h>

using namespace std;


namespace conrad { namespace cp {

  SolverTest::SolverTest()
  {}

  SolverTest::~SolverTest()
  {}

  WorkerProxy::ShPtr SolverTest::create()
  {
    return WorkerProxy::ShPtr (new SolverTest());
  }

  void SolverTest::setInitInfo (const std::string& measurementSet,
				const std::string& inputColumn,
				const std::string& skyParameterDB,
				const std::string& instrumentParameterDB,
				unsigned int subBand,
				bool calcUVW)
  {
    MWCOUT << "SolverTest::setInitInfo" << endl
           << "  MS:         " << measurementSet << endl
           << "  Column:     " << inputColumn  << endl
           << "  SkyParmDB:  " << skyParameterDB << endl
           << "  InstParmDB: " << instrumentParameterDB << endl
           << "  Subband:    " << subBand << endl
           << "  CalcUVW:    " << calcUVW << endl;
  }

  void SolverTest::doProcess (int operation, int streamId,
			      LOFAR::BlobIStream& in,
			      LOFAR::BlobString& out)
  {
    MWCOUT << "SolverTest::doProcess" << endl;
    MWCOUT << "  Operation: " << operation << endl;
    MWCOUT << "  StreamId:  " << streamId << endl;
    switch (operation) {
    case MasterControl::SetWd:
    {
      ObsDomain workDomain;
      in >> workDomain;
      MWCOUT << "  Set work domain: " << workDomain << endl;
      break;
    }
    case MasterControl::Step:
    {
      // A step has to be processed.
      // Only a solve can be processed.
      CONRADCHECK (in.getNextType() == "MWSolveStep",
		   "SolverTest can only handle an MWSolveStep step");
      MWSolveStep step;
      // Fill it from the blobstream.
      step.fromBlob (in);
      itsMaxIter = step.getMaxIter();
      itsNrIter  = 0;
      MWCOUT << "  Solve maxiter " << itsMaxIter << endl;
      break;
    }
    case MasterControl::ParmInfo:
    {
      // ParmInfo has to be processed.
      bool result;
      in >> result;
      MWCOUT << "  ParmInfo " << result << endl;
      break;
    }
    case MasterControl::GetEq:
    {
      // Equations have to be processed.
      bool result;
      in >> result;
      MWCOUT << "  GetEq " << result << endl;
      break;
    }
    case MasterControl::Solve:
    {
      MWCOUT << "  Solve iteration: " << itsNrIter << endl;
      ++itsNrIter;
      bool converged = itsNrIter>=itsMaxIter;
      MWBlobOut bout (out, MasterControl::Solve, streamId);
      bout.blobStream() << converged;
      bout.finish();
      break;
    }
    default:
      CONRADTHROW (MWError, "SolverTest::doProcess: operation "
		   << operation << " is unknown");
    }
  }

}} // end namespaces
