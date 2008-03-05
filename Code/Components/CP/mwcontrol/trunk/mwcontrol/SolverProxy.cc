//#  SolverProxy.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/SolverProxy.h>


namespace askap { namespace cp {

  SolverProxy::~SolverProxy()
  {}

  std::vector<int> SolverProxy::getWorkTypes() const
  {
    return std::vector<int>(1, 1);       // 1 = solver
  }

}} // end namespaces
