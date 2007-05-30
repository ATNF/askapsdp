/// @file
///
/// GenericConverter: A class for measure conversion. This is an
/// implementation of the low-level interface, which is used within the
/// implementation of the data accessor. The end user interacts with the
/// IDataConverter class. This template can be used for all measures, where
/// a subtraction of origin is not required. For MEpoch, where such
/// subtraction is required, there is a separate class EpochConverter
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef GENERIC_CONVERTER_H
#define GENERIC_CONVERTER_H

// CASA includes
#include <measures/Measures/MEpoch.h>

// own includes
#include "IConverterBase.h"

namespace conrad {

namespace synthesis {

/// An implementation of generic converter. This class just
/// call the appropriate functionality of the epoch measures.

template<typename M>
struct GenericConverter : public IConverterBase {
    /// create a converter to the target frame/unit
    /// @param targetRef target reference frame        
    /// @param targetUnit desired units in the output. 
    GenericConverter(const M::Ref &targetRef,
                     const casa::Unit &targetUnit) : itsTargetRef(targetRef),
		           itsTargetUnit(targetUnit) {}

    /// convert specified measure to the target units/frame
    /// @param in a measure to convert. 
    virtual casa::Double operator()(const M &in) const {
       M::MVType converted=M::Convert(in.getRef(),itsTargetRef)(in).getValue();
       return converted.get(itsTargetUnit);
    }
private:    
    M::Ref itsTargetRef;
    casa::Unit  itsTargetUnit;
};

} // namespace synthesis

} // namespace conrad

#endif // GENERIC_CONVERTER_H
