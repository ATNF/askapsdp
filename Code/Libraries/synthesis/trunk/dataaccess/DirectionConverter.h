/// @file
///
/// DirectionConverter: A class for direction conversion. This is an
/// implementation of the low-level interface, which is used within the
/// implementation of the data accessor. The end user interacts with the
/// IDataConverter class. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef DIRECTION_CONVERTER_H
#define DIRECTION_CONVERTER_H

// CASA includes
#include <measures/Measures/MDirection.h>

// own includes
#include "IDirectionConverter.h"

namespace conrad {

namespace synthesis {

/// An implementation of the direction converter. This class just
/// call the appropriate functionality of the direction measures.
/// TODO: we probably need a class where default input frame can be
/// specified at construction (e.g. operator() can receive MVDirection or
/// even Doubles). Such class can be derived from this one
///
struct DirectionConverter : public IDirectionConverter {
    /// create a converter to the target frame
    /// @param targetFrame a desired reference frame
    ///        Class defaults to J2000
    DirectionConverter(const casa::MDirection::Ref &targetFrame);

    /// convert specified MEpoch to the target units/frame
    /// @param in an epoch to convert. 
    virtual casa::MVDirection operator()(const casa::MDirection &in) const;
private:
    casa::MDirection::Ref itsTargetFrame;    
};

} // namespace synthesis

} // namespace conrad

#endif // EPOCH_CONVERTER_H
