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

#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IConstDataAccessor.h>

namespace askap {

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

} // end of namespace askap
