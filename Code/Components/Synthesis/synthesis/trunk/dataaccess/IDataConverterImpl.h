/// @file IDataConverterImpl.h
/// @brief A rich interface to describe on-the-fly conversions
/// @details Interface to describe on-the-fly conversions requested
/// from the data source object. In contrast to IDataConverter, this
/// interface contains methods used within the implementation part of the
/// data access layer, which are not exposed to the end user.
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

#ifndef I_DATA_CONVERTER_IMPL_H
#define I_DATA_CONVERTER_IMPL_H

// boost includes
#include <boost/shared_ptr.hpp>

// CASA includes
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MRadialVelocity.h>

// own includes
#include <dataaccess/IDataConverter.h>

namespace askap {

namespace synthesis {

/// @brief A rich interface to describe on-the-fly conversions (not exposed to end user)
/// @details Interface to describe on-the-fly conversions requested
/// from the data source object. In contrast to IDataConverter, this
/// interface contains methods used within the implementation part of the
/// data access layer, which are not exposed to the end user.
/// @ingroup dataaccess_conv
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
    
    /// @brief Clone the converter (sort of a virtual constructor)
    /// @details The same converter can be used to create a number of iterators.
    /// However, we need to set the reference frame to perform some 
    /// conversions on the per-iterator basis. To avoid very nasty bugs when 
    /// two independent iterators indirectly affect each other by using different
    /// reference frames for conversion, it is practical to isolate all changes
    /// to a private copy of the converter. Each iterator will clone a 
    /// converter in the constructor instead of using the same instance via
    /// a smart pointer. 
    /// @note An alternative approach is to ammend the interface
    /// to pass the reference frame as an argument for methods, which perform
    /// the conversions. Time will show which one is better. A change from one
    /// way of solving the problem to another doesn't affect the high level
    /// user interface and can be done relatively easy.
    /// @return a smart pointer to a clone of this instance of the converter
    virtual boost::shared_ptr<IDataConverterImpl> clone() const = 0;
    
    /// setMeasFrame method is made public for all derived classes
    using IDataConverter::setMeasFrame; 
};


} // namespace synthesis

} // namespace askap
#endif // I_DATA_CONVERTER_IMPL_H
