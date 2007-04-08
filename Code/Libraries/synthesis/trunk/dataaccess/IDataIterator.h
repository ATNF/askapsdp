/// @file IConstDataIterator.h
///
/// IConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// The idea is that an iterator object will be obtained via IDataSource
/// which will take care of the actual method to access the data and the
/// source (a MeasurementSet or a stream). Any class controlling data selection
/// is likely to be held by a real implementation of the iterator. However,
/// it will be set up via the IDataSource object and IS NOT a part of this
/// interface.
/// 
/// Additional read/write buffers can be used via the IDataIterator, which
/// implements a read/write interface
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_CONST_DATA_ITERATOR_H
#define I_CONST_DATA_ITERATOR_H

#include "IDataAccessor.h"

namespace conrad {

namespace synthesis {

class IConstDataIterator
{
public:
	/// an empty virtual destructor to make the compiler happy
	virtual ~IConstDataIterator();
	
	/// Restart the iteration from the beginning
	virtual void init() = 0;
	
	/// Return the data accessor (current chunk) in various ways
	
	/// operator* delivers a reference to data accessor (current chunk)
	/// @return a reference to the current chunk
	virtual const IConstDataAccessor& operator*() const = 0;
	
	/// operator-> delivers a pointer. 
	/// @return a pointer to the current chunk
	/// Allows the syntax like it->visibility()
	/// The default implementation works via operator*, however to 
	/// avoid an additional function call, the method
	/// can be specialized in the derived classes
	virtual const IConstDataAccessor* operator->() const;

	/// Checks whether there are more data available.
	/// @return True if there are more data available
	virtual casa::Bool hasMore() const throw() = 0;

	/// Checks whether the iterator reached an end.
        /// @return True if the iterator has reached an end. 
	/// The Default implementation works via hasMore(), however 
	/// one can override the method in a derived class to avoid 
	/// this (slight) overhead
	virtual casa::Bool atEnd() const throw();

	/// advance the iterator one step further 
	/// @return True if there are more data (so constructions like 
	///         while(it.next()) {} are possible)
	virtual casa::Bool next() = 0;

	/// advance the iterator one step further
	/// @return A reference to itself (to allow ++++it synthax)
	/// The default implementation is via next(), however one can
	/// override this method in a derived class to avoid this (slight)
	/// overhead
	virtual IConstDataIterator& operator++(int);
};

} // end of namespace synthesis

} // end of namespace conrad
#endif // #ifndef I_CONST_DATA_ITERATOR_H
