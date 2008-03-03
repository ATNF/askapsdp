//#  ConradError.cc: Base class for CONRAD exceptions
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <askap/ConradError.h>

namespace conrad {

  ConradError::ConradError(const std::string& message)
    : std::runtime_error(message)
  {}

  ConradError::~ConradError() throw()
  {}


  CheckError::CheckError(const std::string& message)
    : ConradError(message)
  {}

  CheckError::~CheckError() throw()
  {}


  AssertError::AssertError(const std::string& message)
    : ConradError(message)
  {}

  AssertError::~AssertError() throw()
  {}

} // end namespaces
