//# MWError.cc: Basic exception for mwcontrol related errors
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWError.h>

using namespace conrad;

namespace conrad { namespace cp {

  MWError::MWError (const std::string& message)
    : ConradError (message)
  {}

  MWError::~MWError() throw()
  {}

}} // end namespaces
