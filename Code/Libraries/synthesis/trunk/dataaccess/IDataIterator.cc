/// @file
///
/// IDataIterator: Allow iteration across preselected data. Each 
/// iteration step is represented by the MEDataAccessor interface.
/// The idea is that an iterator object will be obtained via MEDataSource
/// which will take care of the actual method to access the data and the
/// source (a MeasurementSet or a stream). Any class controlling data selection
/// is likely to be held by a real implementation of the iterator. However,
/// it will be set up via the MEDataSource object and IS NOT a part of this
/// interface.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include "IDataIterator.h"
#include "IDataAccessor.h"

namespace conrad {

namespace synthesis {

/// IDataIterator is an abstract class defining the interface
/// Only some trivial methods are defined here

/// an empty virtual destructor to make the compiler happy
IDataIterator::~IDataIterator()
{
}
	
/// operator-> delivers a pointer. 
/// @return a pointer to the current chunk
/// Allows the syntax like it->visibility()
/// The default implementation works via operator*, however to 
/// avoid an additional function call, the method
/// can be specialized in the derived classes
const IDataAccessor* IDataIterator::operator->() const
{
  return &(operator*());
}

/// Checks whether the iterator reached an end.
/// @return True if the iterator has reached an end. 
/// The Default implementation works via hasMore(), however 
/// one can override the method in a derived class to avoid 
/// this (slight) overhead
casa::Bool IDataIterator::atEnd() const throw()
{ 
  return !hasMore();
}

/// advance the iterator one step further
/// @return A reference to itself (to allow ++++it synthax)
/// The default implementation is via next(), however one can
/// override this method in a derived class to avoid this (slight)
/// overhead
IDataIterator& IDataIterator::operator++(int)
{
  next();
  return *this;
}

} // end of namespace synthesis

} // end of namespace conrad
