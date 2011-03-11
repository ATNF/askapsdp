/// @file IConverterBase.h
/// @brief A base class for all converter classes.
/// @details
/// IConverterBase: A base class for all converter classes. It doesn't
/// have any useful functionality and is used as a structural unit.
/// The only method defined is a virtual destructor to make the compiler
/// happy and reduce the number of *.cc files for the derived interfaces
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

#ifndef I_CONVERTER_BASE_H
#define I_CONVERTER_BASE_H

// CASA includes
#include <measures/Measures/MeasFrame.h>

namespace askap {

namespace accessors {

/// @brief A base class for all converter classes.
/// @details It doesn't
/// have any useful functionality and is used as a structural unit.
/// The only method defined is a virtual destructor to make the compiler
/// happy and reduce the number of *.cc files for the derived interfaces
/// @ingroup dataaccess_conv
struct IConverterBase {

    /// an empty virtual destructor to keep the compiler happy
    /// for all derived interfaces
    virtual ~IConverterBase();

protected: // the following method(s) are not for a general framework user,
           // but rather for implementation
    /// set a frame (i.e. time and/or position), where the
    /// conversion is performed
    /// @param[in] frame  MeasFrame object (can be constructed from
    ///               MPosition or MEpoch on-the-fly)
    virtual void setMeasFrame(const casa::MeasFrame &frame) = 0;
};

} // namespace accessors

} // namespace askap

#endif // #ifndef I_CONVERTER_BASE_H
