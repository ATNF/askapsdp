//#  BBSProxy.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_BBSPROXY_H
#define CONRAD_MWCONTROL_BBSPROXY_H

#include <mwcommon/WorkerProxy.h>


namespace conrad { namespace cp {

  class BBSProxy: public WorkerProxy
  {
  public:
    BBSProxy()
    {}

    virtual ~BBSProxy();

    virtual void process (int operation, int streamId,
			  LOFAR::BlobIStream& in, LOFAR::BlobString& out);

    virtual void setInitInfo (const std::string& measurementSet,
			      const std::string& inputColumn,
			      const std::string& skyParameterDB,
			      const std::string& instrumentParameterDB,
			      unsigned int subBand,
			      bool calcUVW) = 0;

  private:
    virtual void doProcess (int operation, int streamId,
			    LOFAR::BlobIStream& in,
			    LOFAR::BlobString& out) = 0;
  };

}} /// end namespaces

#endif
