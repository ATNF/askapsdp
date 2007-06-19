//#  PredifferTest.cc: Prediffer BBSler of distributed VDS processing
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include "PredifferTest.h"
#include "MWStepTester.h"
#include "MWIos.h"
#include <mwcommon/MasterControl.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWBlobIO.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobString.h>

using namespace std;


namespace conrad { namespace cp {

  PredifferTest::PredifferTest()
  {}

  PredifferTest::~PredifferTest()
  {}

  WorkerProxy::ShPtr PredifferTest::create()
  {
    return WorkerProxy::ShPtr (new PredifferTest());
  }

  void PredifferTest::setInitInfo (const std::string& measurementSet,
				   const std::string& inputColumn,
				   const std::string& skyParameterDB,
				   const std::string& instrumentParameterDB,
				   unsigned int subBand,
				   bool calcUVW)
  {
    MWCOUT << "PredifferTest::setInitInfo" << endl
           << "  MS:         " << measurementSet << endl
           << "  Column:     " << inputColumn  << endl
           << "  SkyParmDB:  " << skyParameterDB << endl
           << "  InstParmDB: " << instrumentParameterDB << endl
           << "  Subband:    " << subBand << endl
           << "  CalcUVW:    " << calcUVW << endl;
  }

  void PredifferTest::doProcess (int operation, int streamId,
				 LOFAR::BlobIStream& in,
				 LOFAR::BlobString& out)
  {
    MWCOUT << "PredifferTest::doProcess" << endl;
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
      // A step has to be processed; first construct the object.
      MWStep::ShPtr step = MWStepFactory::create (in.getNextType());
      // Fill it from the blobstream.
      step->fromBlob (in);
      // Process the step (using a visitor).
      MWStepTester visitor (streamId, &out);
      step->visit (visitor);
      break;
    }
    case MasterControl::GetEq:
    {
      MWCOUT << "  GetEq" << endl;
      MWBlobOut bout (out, MasterControl::GetEq, streamId);
      bout.blobStream() << true;
      bout.finish();
      break;
    }
    case MasterControl::Solve:
    {
      MWCOUT << "  Solve" << endl;
      bool value;
      in >> value;
      break;
    }
    default:
      CONRADTHROW (MWError, "PredifferTest::doProcess: operation "
		   << operation << " is unknown");
    }
  }

}} // end namespaces
