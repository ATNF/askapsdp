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
/// given antenna and polarisation. 
/// @param[in] ant antenna number (0-based)
/// @param[in] pol index of the polarisation product
/// @return name of the parameter
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


/// @brief obtain a name of the parameter
/// @details This version takes into account beam number
/// @param[in] ant antenna number (0-based)
/// @param[in] beam beam number (0-based)
/// @param[in] pol index of the polarisation product
/// @return name of the parameter
std::string NoXPolGain::paramName(casa::uInt ant, casa::uInt beam, casa::uInt pol)
{
  return paramName(ant,pol)+"."+utility::toString<casa::uInt>(beam);
}


} // namespace synthesis

} // namespace askap
