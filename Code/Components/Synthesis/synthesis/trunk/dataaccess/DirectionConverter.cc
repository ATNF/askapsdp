/// @file DirectionConverter.cc
/// @brief A class for direction conversion
/// @details This is an implementation of the low-level interface,
/// which is used within the implementation of the data accessor.
/// The end user interacts with the  IDataConverter class. 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// CASA includes
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVDirection.h>

// own includes
#include <dataaccess/DirectionConverter.h>

using namespace askap;
using namespace askap::synthesis;
using namespace casa;

/// create a converter to the target frame
/// @param targetFrame a desired reference frame
///        Class defaults to J2000
DirectionConverter::DirectionConverter(const casa::MDirection::Ref
                          &targetFrame) : itsTargetFrame(targetFrame) {}


/// convert specified MEpoch to the target units/frame
/// @param in an epoch to convert. 
casa::MVDirection
    DirectionConverter::operator()(const casa::MDirection &in) const
{
    /// this class is supposed to be used in the most general case, hence
    /// we do all conversions. Specializations can be written for the cases
    /// when either frame or unit conversions are not required
    return MDirection::Convert(in.getRef(),
                             itsTargetFrame)(in).getValue();    
}

/// set a frame (i.e. time and/or position), where the
/// conversion is performed
/// @param frame  MeasFrame object (can be constructed from
///               MPosition or MEpoch on-the-fly)
void DirectionConverter::setMeasFrame(const casa::MeasFrame &frame)
{
  itsTargetFrame.set(frame);
}
