/// @file DirectionConverter.h
/// @brief A class for direction conversion
/// @details
/// DirectionConverter is a class for direction conversion. This is an
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
#include <dataaccess/IDirectionConverter.h>

namespace conrad {

namespace synthesis {

/// @brief An implementation of the direction converter.
/// @details This class just call the appropriate functionality
/// of the direction measures.
/// @todo we probably need a class where default input frame can be
/// specified at construction (e.g. operator() can receive MVDirection or
/// even Doubles). Such class can be derived from this one
///
struct DirectionConverter : public IDirectionConverter {
    /// create a converter to the target frame
    /// @param targetFrame a desired reference frame
    ///        Class defaults to J2000
    DirectionConverter(const casa::MDirection::Ref &targetFrame =
                             casa::MDirection::Ref(casa::MDirection::J2000));

    /// convert specified MEpoch to the target units/frame
    /// @param in an epoch to convert. 
    virtual casa::MVDirection operator()(const casa::MDirection &in) const;

    /// set a frame (i.e. time and/or position), where the
    /// conversion is performed
    /// @param frame  MeasFrame object (can be constructed from
    ///               MPosition or MEpoch on-the-fly)
    virtual void setMeasFrame(const casa::MeasFrame &frame);

private:
    casa::MDirection::Ref itsTargetFrame;    
};

} // namespace synthesis

} // namespace conrad

#endif // EPOCH_CONVERTER_H
