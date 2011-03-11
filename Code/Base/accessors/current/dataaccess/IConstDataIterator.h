/// @file IConstDataIterator.h
/// @brief A read-only iterator across preselected data
/// @details Each 
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
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_CONST_DATA_ITERATOR_H
#define I_CONST_DATA_ITERATOR_H

#include <dataaccess/IDataAccessor.h>

namespace askap {

namespace accessors {

/// @brief Read-only iterator across preselected data
/// @details Each 
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
/// @ingroup dataaccess_i
class IConstDataIterator
{
public:
	/// the type of the value pointed by this iterator
	typedef const IConstDataAccessor& value_type;

	/// the type of the pointer returned by operator->
	/// We can't generally just use value_type * because 
	typedef const IConstDataAccessor* pointer_type;

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
	
	/// advance the iterator one step further 
	/// @return True if there are more data (so constructions like 
	///         while(it.next()) {} are possible)
	virtual casa::Bool next() = 0;

	/// advance the iterator one step further
	/// @return A reference to itself (to allow ++++it syntax)
	/// The default implementation is via next(), however one can
	/// override this method in a derived class to avoid this (slight)
	/// overhead
	virtual IConstDataIterator& operator++();
};

} // end of namespace accessors

} // end of namespace askap
#endif // #ifndef I_CONST_DATA_ITERATOR_H
