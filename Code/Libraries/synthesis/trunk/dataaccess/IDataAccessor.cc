/// @file IDataAccessor.cc
///
/// IDataAccessor: Interface class for read-only access to visibility data
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
//
#include "IConstDataAccessor.h"

namespace conrad {

/// an empty virtual destructor is required to make the
/// compiler happy for all derived classes, which don't 
/// require any special destructor (we have virtual 
/// functions, while a default destructor is not virtual)
synthesis::IConstDataAccessor::~IConstDataAccessor()
{
}

} // end of namespace conrad
