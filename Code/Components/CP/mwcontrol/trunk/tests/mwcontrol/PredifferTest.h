//#  PredifferTest.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef ASKAP_MWCONTROL_PREDIFFERTEST_H
#define ASKAP_MWCONTROL_PREDIFFERTEST_H

#include <mwcontrol/PredifferProxy.h>


namespace askap { namespace cp {

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

    virtual int doProcess (int operation, int streamId,
                           LOFAR::BlobIStream& in,
                           LOFAR::BlobOStream& out);
  };

}} // end namespaces

#endif
