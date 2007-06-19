//# WorkerFactory.cc: Factory pattern to generate a workerproxy object
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/WorkerFactory.h>
#include <mwcommon/MWError.h>

namespace conrad { namespace cp {

  void WorkerFactory::push_back (const std::string& name, Creator* creator)
  {
    itsMap[name] = creator;
  }
    
  WorkerProxy::ShPtr WorkerFactory::create (const std::string& name) const
  {
    std::map<std::string,Creator*>::const_iterator iter = itsMap.find(name);
    CONRADCHECK (iter != itsMap.end(),
		 "WorkerProxy " << name << " is unknown");
    return (*iter->second)();
  }

}} // end namespaces
