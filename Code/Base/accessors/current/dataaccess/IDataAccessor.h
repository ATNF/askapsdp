/// @file IDataAccessor.h
/// @brief Interface class to access buffered visibility data
/// @details
/// IDataAccessor: Interface class to access buffered visibility data
///        with the writing permission. It is ment to be used in conjunction 
///        with a read/write iterator (IDataIterator) for an access to 
///        associated buffers and optionally to update the visibilities if
///        the corresponding DataSource allows such operation.
///        
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
#ifndef I_DATA_ACCESSOR_H
#define I_DATA_ACCESSOR_H

#include <dataaccess/IConstDataAccessor.h>

namespace askap {

namespace accessors {

/// @brief an interface class to access buffered visibility data
/// with a read/write capability.
/// @details It is meant to be used together with 
/// derived iterators, which support a read/write access (e.g. for buffers 
/// associated with visibility chunks).
/// @ingroup dataaccess_i
class IDataAccessor : virtual public IConstDataAccessor
{
public:
	
    /// Read-only visibilities (a cube is nRow x nChannel x nPol; 
	/// each element is a complex visibility)
	///
	/// @return a reference to nRow x nChannel x nPol cube, containing
	/// all visibility data
	///
	virtual const casa::Cube<casa::Complex>& visibility() const = 0;

	
    /// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
	/// each element is a complex visibility)
	///
	/// @return a reference to nRow x nChannel x nPol cube, containing
	/// all visibility data
	///
	virtual casa::Cube<casa::Complex>& rwVisibility() = 0;
};

} // end of namespace accessors

} // end of namespace askap

#endif // #ifndef I_DATA_ACCESSOR_H
