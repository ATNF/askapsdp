/// @file
///
/// IEqDataIterator: Allow iteration across preselected data. Each 
/// iteration step is represented by the IEqDataAccessor interface.
/// The idea is that an iterator object will be obtained via IEqDataSource
/// which will take care of the actual method to access the data and the
/// source (a MeasurementSet or a stream). Any class controlling data selection
/// is likely to be held by a real implementation of the iterator. However,
/// it will be set up via the IEqDataSource object and IS NOT a part of this
/// interface.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef IEQDATAITERATOR_H_
#define IEQDATAITERATOR_H_

#include "IEqDataAccessor.h"

namespace conrad {
class IEqDataIterator
{
public:
	/// an empty virtual destructor to make the compiler happy
	virtual ~IEqDataSource();
	
	/// Restart the iteration from the beginning
	virtual void init() = 0;
	
	/// Return the data accessor (current chunk) in various ways
	
	/// operator* delivers a reference
	virtual const IEqDataAccessor& operator*() const = 0;
	
	/// operator-> delivers a pointer. Default implementation works
	/// via operator*, however to avoid an extra function call it also
	/// can be specialized in the derived classes
	virtual const IEqDataAccessor* operator->() const;

	/// True if there is more data available
	virtual casa::Bool hasMore() const throw() = 0;

        /// True if the iterator reached the end. Default implementation is
	/// via hasMore(), however one can override the method in a
	/// derived class to avoid this (slight) overhead
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
	virtual IEqDataIterator& operator++(int);
};
}
#endif /*IEQDATAITERATOR_H_*/
