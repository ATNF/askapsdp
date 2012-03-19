/// @file
/// @brief An read/write interface to flag and noise information
/// @details IFlagAndNoiseDataAccessor is an interface class to access
///        visibility data with the writing permission. This class is a 
///        further extension of IDataAccessor to provide a read/write 
///        interface to both flag and noise information. The user should 
///        dynamic cast to this interface from the reference or pointer 
///        returned by IDataIterator interface.
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
#ifndef I_FLAG_AND_NOISE_DATA_ACCESSOR_H
#define I_FLAG_AND_NOISE_DATA_ACCESSOR_H

#include <dataaccess/IFlagDataAccessor.h>

namespace askap {

namespace accessors {

/// @brief An read/write interface to flag and noise information
/// @details IFlagAndNoiseDataAccessor is an interface class to access
/// visibility data with the writing permission. This class is a 
/// further extension of IDataAccessor to provide a read/write 
/// interface to both flag and noise information. The user should 
/// dynamic cast to this interface from the reference or pointer 
/// returned by IDataIterator interface.
/// @ingroup dataaccess_i
class IFlagAndNoiseDataAccessor : virtual public IFlagDataAccessor
{
public:
    /// @brief Noise level required for a proper weighting
	/// @return a reference to nRow x nChannel x nPol cube with
	///         complex noise estimates. Elements correspond to the
	///         visibilities in the data cube.
	virtual const casa::Cube<casa::Complex>& noise() const = 0;

    /// @brief write access to Noise level 
	/// @return a reference to nRow x nChannel x nPol cube with
	///         complex noise estimates. Elements correspond to the
	///         visibilities in the data cube.
	virtual casa::Cube<casa::Complex>& rwNoise() = 0;

};

} // end of namespace accessors

} // end of namespace askap

#endif // #ifndef I_FLAG_AND_NOISE_DATA_ACCESSOR_H
