/// @file IDirectionConverter.h
/// @brief An interface for direction conversion.
/// @details This is a
/// relatively low-level interface, which is used within the implementation
/// of the data accessor. The end user interacts with the IDataConverter
/// class. 
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

#ifndef I_DIRECTION_CONVERTER_H
#define I_DIRECTION_CONVERTER_H

// CASA includes
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/MVDirection.h>

// own includes
#include <dataaccess/IConverterBase.h>

namespace askap {

namespace accessors {

/// @brief An interface for direction conversion.
/// @details This is a
/// relatively low-level interface, which is used within the implementation
/// of the data accessor. The end user interacts with the IDataConverter
/// class. 
/// @ingroup dataaccess_conv
struct IDirectionConverter : virtual public IConverterBase {
    /// convert specified MDirection to the target frame
    /// @param in an epoch to convert. Target frame is a
    /// property of the actual instance of the derived class
    virtual casa::MVDirection operator()(const casa::MDirection &in) const = 0;

    /// using statement to have setMeasFrame public.
    using IConverterBase::setMeasFrame;
};

} // namespace accessors

} // namespace askap

#endif // I_DIRECTION_CONVERTER_H
