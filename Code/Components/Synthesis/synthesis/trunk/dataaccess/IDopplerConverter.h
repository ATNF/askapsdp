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
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
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

namespace synthesis {

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

} // namespace synthesis

} // namespace askap

#endif // I_DOPPLER_CONVERTER_H
