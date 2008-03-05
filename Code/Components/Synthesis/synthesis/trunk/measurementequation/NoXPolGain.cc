/// @file
/// 
/// @brief Calibration effect: antenna gains without cross-pol
/// @details This is a simple effect which can be used in conjunction
/// with the CalibrationME template (as its template argument)
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>


#include <measurementequation/NoXPolGain.h>
#include <askap/AskapUtil.h>
#include <askap/AskapError.h>


namespace askap {

namespace synthesis {


/// @brief obtain a name of the parameter
/// @details This method returns the parameter name for a gain of the
/// given antenna and polarisation. In the future, we may add time and/or
/// feed number as well.
/// @param[in] ant antenna number (0-based)
/// @param[in] pol index of the polarisation product
std::string NoXPolGain::paramName(casa::uInt ant, casa::uInt pol)
{
  std::string res("gain.");
  if (!pol) {
      res+="g11.";
  } else if (pol == 1) {
      res+="g22.";
  } else {
      ASKAPTHROW(AskapError, "Only parallel hand polarisation products are supported at the moment, you have pol="<<pol);
  }

  return res+utility::toString<casa::uInt>(ant);
}

/// @brief obtain polarisation indices
/// @details We really need a better way of handling orders of polarisation
/// products. I hope this method is temporary. It translates polarisation
/// plane number in the visibility cube to two polarisation indices (i.e. 0 or 1).
/// @param[in] pol polarisation plane number in the visibility cube 
/// @param[in] nPol total number of polarisation planes
/// @return a pair with polarisation indices
std::pair<casa::uInt, casa::uInt> NoXPolGain::polIndices(casa::uInt pol, casa::uInt nPol)
{
  ASKAPDEBUGASSERT((nPol == 1) || (nPol == 2) || (nPol == 4));
  ASKAPDEBUGASSERT(pol<nPol);
  if (nPol < 4) {
      // no special polarisation handling is required (either a single plane
      // or two planes corresponding to the orthogonal polarisations)
      return std::make_pair(pol,pol); 
  }
  const casa::uInt firstIndices[4] = {0,0,1,1};
  const casa::uInt secondIndices[4] = {0,1,0,1};
  return std::make_pair(firstIndices[pol], secondIndices[pol]);
}

} // namespace synthesis

} // namespace askap
