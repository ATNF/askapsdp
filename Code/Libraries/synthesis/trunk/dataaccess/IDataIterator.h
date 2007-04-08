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

#ifndef I_DATA_ITERATOR_H
#define I_DATA_ITERATOR_H

#include "IConstDataIterator.h"
#include "IDataAccessor.h"

namespace conrad {

namespace synthesis {

class IDataIterator : public IConstDataIterator
{
public:
	
	/// Return the data accessor (current chunk) in various ways
	
	/// operator* delivers a reference to data accessor (current chunk)
	///
	/// @return a reference to the current chunk
	///
	/// constness of the return type is changed to allow read/write
	/// operations.
	///
	virtual IDataAccessor& operator*() const = 0;
	
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
	virtual IDataAccessor* operator->() const;

	/// return any associated buffer for read/write access
	/// 
	/// @param[in] bufferID the number of the buffer requested
	/// @return a reference to writable data accessor to the
	///         buffer requested
	///
	virtual IConstDataAccessor& buffer(casa::uInt bufferID) const = 0;

	/// advance the iterator one step further
	///
	/// @return A reference to itself (to allow ++++it synthax)
	///
	/// The default implementation is via next(), however one can
	/// override this method in a derived class to avoid this (slight)
	/// overhead. This method overrides the the method of the base
	/// class to return the correct type 
	virtual IDataIterator& operator++(int);
};

} // end of namespace synthesis

} // end of namespace conrad
#endif // #ifndef I_DATA_ITERATOR_H
