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

// CASA includes
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVDirection.h>

// own includes
#include "DirectionConverter.h"

using namespace conrad;
using namespace synthesis;
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
