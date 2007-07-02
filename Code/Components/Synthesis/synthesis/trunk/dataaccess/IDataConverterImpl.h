/// @file IDataConverterImpl.h
/// @brief A rich interface to describe on-the-fly conversions
/// @details Interface to describe on-the-fly conversions requested
/// from the data source object. In contrast to IDataConverter, this
/// interface contains methods used within the implementation part of the
/// data access layer, which are not exposed to the end user.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_DATA_CONVERTER_IMPL_H
#define I_DATA_CONVERTER_IMPL_H

// CASA includes
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MRadialVelocity.h>

// own includes
#include <dataaccess/IDataConverter.h>

namespace conrad {

namespace synthesis {

/// @brief A rich interface to describe on-the-fly conversions (not exposed to end user)
/// @details Interface to describe on-the-fly conversions requested
/// from the data source object. In contrast to IDataConverter, this
/// interface contains methods used within the implementation part of the
/// data access layer, which are not exposed to the end user.
/// @ingroup dataaccess
class IDataConverterImpl : virtual public IDataConverter
{
public:
    /// convert epochs
    /// @param[in] in input epoch given as an MEpoch object
    /// @return epoch converted to Double 
    virtual casa::Double epoch(const casa::MEpoch &in) const = 0;

    /// reverse conversion: form a measure from 'double' epoch
    /// @param[in] in epoch given as Double in the target units/frame
    /// @return epoch converted to Measure
    virtual casa::MEpoch epochMeasure(casa::Double in) const = 0;

    /// reverse conversion: form a measure from MVEpoch
    /// @param[in] in epoch given as MVEpoch in the target frame
    /// @return epoch converted to Measure
    virtual casa::MEpoch epochMeasure(const casa::MVEpoch &in) const = 0;

    /// convert directions
    /// @param[in] in input direction given as an MDirection object
    /// @param out output direction as an MVDirection object
    virtual void direction(const casa::MDirection &in,
                           casa::MVDirection &out) const = 0;

    /// test whether the frequency conversion is void
    /// @param[in] testRef reference frame to test
    /// @param[in] testUnit units to test
    virtual bool isVoid(const casa::MFrequency::Ref &testRef,
                        const casa::Unit &testUnit) const = 0;

    /// convert frequencies
    /// @param[in] in input frequency given as an MFrequency object
    /// @return output frequency as a Double
    virtual casa::Double frequency(const casa::MFrequency &in) const = 0;

    /// convert velocities
    /// @param[in] in input velocities given as an MRadialVelocity object
    /// @return output velocity as a Double
    virtual casa::Double velocity(const casa::MRadialVelocity &in) const = 0;

    /// convert frequencies from velocities
    /// @param[in] in input velocity given as an MRadialVelocity object
    /// @return output frequency as a Double
    ///
    /// @note An exception will be thrown if the rest frequency is not
    /// defined.
    ///
    virtual casa::Double frequency(const casa::MRadialVelocity &in) const = 0;

    /// convert velocities from frequencies
    /// @param[in] in input frequency  given as an MFrequency object
    /// @return output velocity as a Double
    ///
    /// @note An exception will be thrown if the rest frequency is not
    /// defined.
    ///
    virtual casa::Double velocity(const casa::MFrequency &in) const = 0;
};


} // namespace synthesis

} // namespace conrad
#endif // I_DATA_CONVERTER_IMPL_H
