/// @file IFlagDataAccessor.h
/// @brief An read/write interface to flagging information
/// @details  IFlagDataAccessor is an interface class to access
///        buffered visibility data
///        with the writing permission. This class is a further extension
///        of IDataAccessor to provide a read/write interface to the 
///        flag information. The user should dynamic cast to this 
///        interface from the reference or pointer returned by  
///        IDataIterator interface.
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
#ifndef I_FLAG_DATA_ACCESSOR_H
#define I_FLAG_DATA_ACCESSOR_H

#include <dataaccess/IDataAccessor.h>

namespace askap {

namespace synthesis {

/// @brief An read/write interface to flagging information
/// @details IFlagDataAccessor is a further extension of the
/// IDataAccessor interface class to provide a read/write access to
/// the flag information.
/// @ingroup dataaccess_i
class IFlagDataAccessor : virtual public IDataAccessor
{
public:
        /// Cube of flags corresponding to the output of visibility()
        /// @return a reference to nRow x nChannel x nPol cube with the flag
        ///         information. If True, the corresponding element is flagged.
        virtual const casa::Cube<casa::Bool>& flag() const = 0;

	/// Non-const access to the cube of flags.
        /// @return a reference to nRow x nChannel x nPol cube with the flag
        ///         information. If True, the corresponding element is flagged.
        virtual casa::Cube<casa::Bool>& rwFlag() = 0;
};

} // end of namespace synthesis

} // end of namespace askap

#endif // #ifndef I_FLAG_DATA_ACCESSOR_H
