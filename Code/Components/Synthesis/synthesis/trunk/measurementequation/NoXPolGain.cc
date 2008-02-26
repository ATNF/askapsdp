/// @file
/// 
/// @brief Calibration effect: antenna gains without cross-pol
/// @details This is a simple effect which can be used in conjunction
/// with the CalibrationME template (as its template argument)
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>


#include <measurementequation/NoXPolGain.h>
#include <conrad/ConradUtil.h>
#include <conrad/ConradError.h>


namespace conrad {

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
      CONRADTHROW(ConradError, "Only parallel hand polarisation products are supported at the moment, you have pol="<<pol);
  }

  return res+utility::toString<casa::uInt>(ant);
}


/// @brief obtain polarisation index
/// @details We really need a better way of handling orders of polarisation
/// products. I hope this method is temporary. It translates polarisation
/// plane number in the visibility cube to polarisation index (i.e. 0 or 1).
/// The method returns nPol, if this polarisation corresponds to a 
/// cross-product.
/// @param[in] pol polarisation plane number in the visibility cube 
/// @param[in] nPol total number of polarisation planes
/// @return polarisation index
casa::uInt NoXPolGain::polIndex(casa::uInt pol, casa::uInt nPol)
{
  CONRADDEBUGASSERT((nPol == 1) || (nPol == 2) || (nPol == 4));
  CONRADDEBUGASSERT(pol<nPol);
  if (nPol < 4) {
      return pol; // no polarisation handling is required (either single plane
                  // or two planes corresponding to two orthogonal polarisations)
  }
  // full polarisations case
  if ((pol == 1) || (pol == 2)) {
      return nPol; // these are cross-products - exclude them
  } 
  return (pol == 3) ? 1 : 0;
}

} // namespace synthesis

} // namespace conrad
