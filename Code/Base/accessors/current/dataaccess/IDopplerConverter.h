/// @file IDopplerConverter.h
/// @brief An interface for interconversion between frequencies
/// and velocities.
/// @details This is a relatively low-level interface, which is
/// used within the implementation of the data accessor. The end user
/// interacts with the IDataConverter class only.
///
/// The idea behind this class is very similar to CASA's VelocityMachine,
/// but we require a bit different interface to use the class efficiently
/// (and the interface conversion would be equivalent in complexity to
/// the transformation itself). Hence, we will use a class derived from
/// this interface instead of the VelocityMachine
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

#ifndef I_DOPPLER_CONVERTER_H
#define I_DOPPLER_CONVERTER_H

// CASA includes
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MRadialVelocity.h>

// own includes
#include <dataaccess/IConverterBase.h>

namespace askap {

namespace accessors {

/// @brief An interface for interconversion between frequencies
/// and velocities.
/// @details This is a relatively low-level interface, which is
/// used within the implementation of the data accessor. The end user
/// interacts with the IDataConverter class only.
///
/// The idea behind this class is very similar to CASA's VelocityMachine,
/// but we require a bit different interface to use the class efficiently
/// (and the interface conversion would be equivalent in complexity to
/// the transformation itself). Hence, we will use a class derived from
/// this interface instead of the VelocityMachine
/// @ingroup dataaccess_conv
struct IDopplerConverter : virtual public IConverterBase {
    /// convert specified frequency to velocity in the same reference
    /// frame. Velocity definition (i.e. optical or radio, etc) is
    /// determined by the implementation class.
    ///
    /// @param[in] freq an MFrequency measure to convert.
    /// @return a reference on MRadialVelocity object with the result
    virtual const casa::MRadialVelocity& operator()(const casa::MFrequency &freq) const = 0;

    /// convert specified velocity to frequency in the same reference
    /// frame. Velocity definition (i.e. optical or radio, etc) is
    /// determined by the implementation class.
    ///
    /// @param[in] vel an MRadialVelocity measure to convert.
    /// @return a reference on MFrequency object with the result
    virtual const casa::MFrequency&  operator()(const casa::MRadialVelocity &vel) const = 0;
};

} // namespace accessors

} // namespace askap

#endif // I_DOPPLER_CONVERTER_H
