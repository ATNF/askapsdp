/// @file 
///
/// @brief data monitored externally
/// @details This is a basic structure containing a number of monitoring points
/// such as amplitudes, delays or phases. A structure of this type is passed to 
/// a registered call back method at every new correlation cycle. Although we 
/// could've passed the CorrProducts structure which is used in the generic monitoring
/// interface, an adapter seems worth while to avoid a tight coupling between epics part
/// and the rest of the software correlator. In addition, we can latter add other information
/// to the type which is not present in the CorrProducts structure.
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

#include <corrinterfaces/MonitoringData.h>
#include <casa/Quanta/MVEpoch.h>

#include <sstream>

namespace askap {

namespace swcorrelator {

/// @brief constructor, initialises the beam number and resizes the vectors
/// @param[in] beam beam index [0..nBeam-1]
MonitoringData::MonitoringData(const int beam) : itsBeam(beam), itsAmplitudes(3,0.),
      itsPhases(3,0.), itsDelays(3,0.), itsFlags(3,true), itsTime(0.) {}

/// @brief obtain UT date/time string
/// @return the date/time corresponding to itsTime as a string (to simplify reporting)
std::string MonitoringData::timeString() const
{
  const casa::MVEpoch epoch(itsTime);
  std::ostringstream os;
  epoch.print(os);
  return os.str();
}

} // namespace swcorrelator

} // namespace askap

