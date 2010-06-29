/// @file
/// 
/// @brief Calibration effect: polarisation leakage
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


#include <measurementequation/LeakageTerm.h>
#include <askap/AskapUtil.h>
#include <askap/AskapError.h>


namespace askap {

namespace synthesis {

/// @brief obtain a name of the parameter
/// @details This method produces a parameter name in the form d12.x.y or d21.x.y depending on the
/// polarisation index (12 or 21), antenna (x) and beam (y) ids.
/// @param[in] ant antenna number (0-based)
/// @param[in] beam beam number (0-based)
/// @param[in] pol12 index of the polarisation product (12 == true or 21 == false)
/// @return name of the parameter
std::string LeakageTerm::paramName(casa::uInt ant, casa::uInt beam, bool pol12)
{
  std::string term = "d21.";
  if (pol12) {
      term = "d12.";
  }
  return term+utility::toString<casa::uInt>(ant)+"."+utility::toString<casa::uInt>(beam);
}


} // namespace synthesis

} // namespace askap
