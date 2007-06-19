//#  SolverProxy.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_SOLVERPROXY_H
#define CONRAD_MWCONTROL_SOLVERPROXY_H

#include <mwcontrol/BBSProxy.h>


namespace conrad { namespace cp {

  class SolverProxy: public BBSProxy
  {
  public:
    SolverProxy()
    {}

    virtual ~SolverProxy();

    /// Get the work types supported by the proxy.
    virtual std::vector<int> getWorkTypes() const;
  };

}} /// end namespaces

#endif
