//# MWError.cc: Basic exception for mwcontrol related errors
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWError.h>

using namespace askap;

namespace askap { namespace cp {

  MWError::MWError (const std::string& message)
    : AskapError (message)
  {}

  MWError::~MWError() throw()
  {}

}} // end namespaces
