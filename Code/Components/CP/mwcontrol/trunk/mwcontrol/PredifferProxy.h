//#  PredifferProxy.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_PREDIFFERPROXY_H
#define CONRAD_MWCONTROL_PREDIFFERPROXY_H

#include <mwcontrol/BBSProxy.h>
#include <boost/shared_ptr.hpp>


namespace conrad { namespace cp {

  class PredifferProxy: public BBSProxy
  {
  public:
    PredifferProxy()
    {}

    ~PredifferProxy();

    /// Get the work types supported by the proxy.
    virtual std::vector<int> getWorkTypes() const;
  };

}} /// end namespaces

#endif
