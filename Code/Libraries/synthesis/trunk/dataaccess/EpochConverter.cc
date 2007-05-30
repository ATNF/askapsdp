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

/// CASA includes
#include <measures/Measures/MCEpoch.h>
#include <casa/Quanta/MVTime.h>
#include <measures/Measures/MeasConvert.h>

/// own includes
#include <dataaccess/EpochConverter.h>


using namespace conrad;
using namespace synthesis;
using namespace casa;

/// create a converter to the target frame/unit
/// @param targetOrigin a measure describing target reference frame
///        and origin (e.g. w.r.t. midnight 30/05/2007 UTC)
///        Class defaults to MJD 0 UTC
/// @param targetUnit desired units in the output. Class defaults
///        to seconds
EpochConverter::EpochConverter(const casa::MEpoch &targetOrigin,
                       const casa::Unit &targetUnit) :
        itsTargetOrigin(targetOrigin), itsTargetUnit(targetUnit) {}

/// convert specified MEpoch to the target units/frame
/// @param in an epoch to convert. 
casa::Double EpochConverter::operator()(const casa::MEpoch &in) const
{
  /// this class is supposed to be used in the most general case, hence
  /// we do all conversions. Specializations can be written for the cases
  /// when either frame or unit conversions are not required
  MVEpoch converted=MEpoch::Convert(in.getRef(),
                        itsTargetOrigin.getRef())(in).getValue();
  // relative to the origin
  converted-=itsTargetOrigin.getValue();
  return converted.getTime(itsTargetUnit).getValue();
}
