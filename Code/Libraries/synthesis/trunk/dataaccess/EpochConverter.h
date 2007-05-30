/// @file
///
/// EpochConverter: A class for epoch conversion. This is an implementation
/// of the low-level interface, which is used within the implementation of
/// the data accessor. The end user interacts with the IDataConverter
/// class. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef EPOCH_CONVERTER_H
#define EPOCH_CONVERTER_H

// CASA includes
#include <measures/Measures/MEpoch.h>

// own includes
#include "IEpochConverter.h"

namespace conrad {

namespace synthesis {

/// An implementation of the epoch converter. This class just
/// call the appropriate functionality of the epoch measures.
/// TODO: we probably need a class where default input frame can be
/// specified at construction (e.g. operator() can receive MVEpoch or
/// even a Double). Such class can be derived from this one
///
struct EpochConverter : public IEpochConverter {
    /// create a converter to the target frame/unit
    /// @param targetOrigin a measure describing target reference frame
    ///        and origin (e.g. w.r.t. midnight 30/05/2007 UTC)
    ///        Class defaults to MJD 0 UTC
    /// @param targetUnit desired units in the output. Class defaults
    ///        to seconds
    EpochConverter(const casa::MEpoch &targetOrigin = casa::MEpoch(),
                   const casa::Unit &targetUnit = "s");

    /// convert specified MEpoch to the target units/frame
    /// @param in an epoch to convert. 
    virtual casa::Double operator()(const casa::MEpoch &in) const;
private:
    casa::MEpoch itsTargetOrigin;
    casa::Unit  itsTargetUnit;
};

} // namespace synthesis

} // namespace conrad

#endif // EPOCH_CONVERTER_H
