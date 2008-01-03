/// @file IConstDataIterator.cc
/// @brief A read-only iterator across preselected data.
/// @details Each  iteration step is represented by the IDataAccessor
/// interface. The idea is that an iterator object will be obtained via
/// IDataSource which will take care of the actual method to access the
/// data and the source (a MeasurementSet or a stream). Any class
/// controlling data selection is likely to be held by a real
/// implementation of the iterator. However,
/// it will be set up via the IDataSource object and IS NOT a part of this
/// interface.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IConstDataAccessor.h>

namespace conrad {

namespace synthesis {

/// IDataIterator is an abstract class defining the interface
/// Only some trivial methods are defined here

/// an empty virtual destructor to make the compiler happy
IConstDataIterator::~IConstDataIterator()
{
}
	
/// operator-> delivers a pointer. 
/// @return a pointer to the current chunk
/// Allows the syntax like it->visibility()
/// The default implementation works via operator*, however to 
/// avoid an additional function call, the method
/// can be specialized in the derived classes
const IConstDataAccessor* IConstDataIterator::operator->() const
{
  return &(operator*());
}

/// advance the iterator one step further
/// @return A reference to itself (to allow ++++it synthax)
/// The default implementation is via next(), however one can
/// override this method in a derived class to avoid this (slight)
/// overhead
IConstDataIterator& IConstDataIterator::operator++()
{
  next();
  return *this;
}

} // end of namespace synthesis

} // end of namespace conrad
