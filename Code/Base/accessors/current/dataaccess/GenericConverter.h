/// @file GenericConverter.h
/// @brief A template for measure conversion.
/// @details GenericConverter is an implementation of the low-level
/// interface, which is used only within the implementation of the data
/// accessor. The end user interacts with the
/// IDataConverter class. This template can be used for all measures, where
/// a subtraction of origin is not required. For MEpoch, where such
/// subtraction is required, there is a separate class EpochConverter
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

#ifndef GENERIC_CONVERTER_H
#define GENERIC_CONVERTER_H

// CASA includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>

// own includes
#include <dataaccess/IConverterBase.h>

namespace askap {

namespace synthesis {

/// @brief An implementation of generic measure converter.
/// @details GenericConverter is an implementation of the low-level
/// interface, which is used only within the implementation of the data
/// accessor. The end user interacts with the
/// IDataConverter class. This template can be used for all measures, where
/// a subtraction of origin is not required. For MEpoch, where such
/// subtraction is required, there is a separate class EpochConverter
/// This class just call the appropriate functionality of the epoch measures.
/// @ingroup dataaccess_conv
template<typename M>
struct GenericConverter : virtual public IConverterBase {
    /// create a converter to the target frame/unit
    /// @param[in] targetRef target reference frame        
    /// @param[in] targetUnit desired units in the output. 
    GenericConverter(const typename M::Ref &targetRef,
                     const casa::Unit &targetUnit) : itsTargetRef(targetRef),
		           itsTargetUnit(targetUnit) {}

    /// convert specified measure to the target units/frame
    /// @param[in] in a measure to convert. 
    virtual inline casa::Double operator()(const M &in) const {       
       typename M::MVType converted=
                typename M::Convert(in.getRef(),itsTargetRef)(in).getValue();
       return converted.get(itsTargetUnit).getValue();
    }

    /// set a frame (i.e. time and/or position), where the
    /// conversion is performed
    /// @param[in] frame  MeasFrame object (can be constructed from
    ///               MPosition or MEpoch on-the-fly)
    virtual void setMeasFrame(const casa::MeasFrame &frame) {
       itsTargetRef.set(frame);
    }

    /// @brief Test whether the given frame and units would cause a
    /// void transformation
    /// @details 
    /// @param[in] testRef reference frame of the propsed input
    /// @param[in] testUnit units in the proposed input
    virtual inline bool isVoid(const typename M::Ref &testRef,
                     const casa::Unit &testUnit) const {		     
       return (itsTargetRef.getType() == testRef.getType()) &&
              (itsTargetUnit.getName() == testUnit.getName());
    }
    

private:    
    typename M::Ref itsTargetRef;
    casa::Unit  itsTargetUnit;
};

} // namespace synthesis

} // namespace askap

#endif // GENERIC_CONVERTER_H
