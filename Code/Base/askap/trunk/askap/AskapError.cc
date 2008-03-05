//#  AskapError.cc: Base class for ASKAP exceptions
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <askap/AskapError.h>

namespace askap {

  AskapError::AskapError(const std::string& message)
    : std::runtime_error(message)
  {}

  AskapError::~AskapError() throw()
  {}


  CheckError::CheckError(const std::string& message)
    : AskapError(message)
  {}

  CheckError::~CheckError() throw()
  {}


  AssertError::AssertError(const std::string& message)
    : AskapError(message)
  {}

  AssertError::~AssertError() throw()
  {}

} // end namespaces
