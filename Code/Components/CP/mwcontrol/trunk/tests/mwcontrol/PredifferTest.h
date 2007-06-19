//#  PredifferTest.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_PREDIFFERTEST_H
#define CONRAD_MWCONTROL_PREDIFFERTEST_H

#include <mwcontrol/PredifferProxy.h>


namespace conrad { namespace cp {

  class PredifferTest: public PredifferProxy
  {
  public:
    PredifferTest();

    ~PredifferTest();

    // Create a new object.
    static WorkerProxy::ShPtr create();

    virtual void setInitInfo (const std::string& measurementSet,
			      const std::string& inputColumn,
			      const std::string& skyParameterDB,
			      const std::string& instrumentParameterDB,
			      unsigned int subBand,
			      bool calcUVW);

    virtual void doProcess (int operation, int streamId,
			    LOFAR::BlobIStream& in,
			    LOFAR::BlobString& out);
  };

}} // end namespaces

#endif
