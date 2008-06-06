/// @file
/// 
/// @brief Calibration effect: antenna gains without cross-pol
/// @details This is a simple effect which can be used in conjunction
/// with the CalibrationME template (as its template argument)
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
