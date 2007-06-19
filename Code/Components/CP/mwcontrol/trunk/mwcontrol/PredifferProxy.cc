//#  PredifferProxy.cc: Prediffer proxyler of distributed VDS processing
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/PredifferProxy.h>


namespace conrad { namespace cp {

  PredifferProxy::~PredifferProxy()
  {}


  std::vector<int> PredifferProxy::getWorkTypes() const
  {
    return std::vector<int>(1, 0);       // 1 = prediffer
  }

}} // end namespaces
