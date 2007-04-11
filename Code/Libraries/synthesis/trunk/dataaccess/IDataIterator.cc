/// @file
///
/// IDataIterator: an interface to the data iterator with
/// associated buffers. See the description of IConstDataIterator
/// for more details. Buffers are the visibility chunks conformant
/// to the data pointed to by the IDataIterator, but with a read/write access. 
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include "IDataIterator.h"

namespace conrad {

namespace synthesis {

/// operator-> delivers a pointer. 
///
/// @return a pointer to the current chunk
///
/// Allows the syntax like it->visibility()
/// The default implementation works via operator*, however to 
/// avoid an additional function call, the method
/// can be specialized in the derived classes
///
/// constness of the return type is changed to allow read/write
/// operations.
///
IDataAccessor* IDataIterator::operator->() const
{
  return &(operator*());
}

/// advance the iterator one step further
///
/// @return A reference to itself (to allow ++++it synthax)
///
/// The default implementation is via next(), however one can
/// override this method in a derived class to avoid this (slight)
/// overhead. This method overrides the the method of the base
/// class to return the correct type 
IDataIterator& IDataIterator::operator++()
{
  next();
  return *this;
}

} // end of namespace synthesis

} // end of namespace conrad
