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

#ifndef ASKAP_CORRINTERFACES_MONITORING_DATA_H
#define ASKAP_CORRINTERFACES_MONITORING_DATA_H

// std includes
#include <vector>
#include <string>

// boost includes
#include <boost/utility.hpp>

namespace askap {

namespace swcorrelator {

/// @brief data monitored externally
/// @details This is a basic structure containing a number of monitoring points
/// such as amplitudes, delays or phases. A structure of this type is passed to 
/// a registered call back method at every new correlation cycle. Although we 
/// could've passed the CorrProducts structure which is used in the generic monitoring
/// interface, an adapter seems worth while to avoid a tight coupling between epics part
/// and the rest of the software correlator. In addition, we can latter add other information
/// to the type which is not present in the CorrProducts structure.
/// @ingroup corrinterfaces
struct MonitoringData : private boost::noncopyable {
  /// @brief individual baselines
  enum Baseline {
     BASELINE_12 = 0,
     BASELINE_23,
     BASELINE_13
  };

  /// @brief constructor, initialises the beam number and resizes the vectors
  /// @param[in] beam beam index [0..nBeam-1]
  explicit MonitoringData(const int beam);
  
  /// @brief obtain UT date/time string
  /// @return the date/time corresponding to itsTime as a string (to simplify reporting)
  std::string timeString() const;
  
  /// @brief the beam number related to this structure
  /// @return the beam number
  int beam() const; 
  
  /// @brief obtain amplitude for a given baseline 
  /// @param[in] baseline baseline index
  /// @return amplitude in raw counts
  float amplitude(const Baseline baseline) const;

  /// @brief obtain phase for a given baseline 
  /// @param[in] baseline baseline index
  /// @return phase in degrees
  float phase(const Baseline baseline) const;

  /// @brief obtain fitted delay for a given baseline 
  /// @param[in] baseline baseline index
  /// @return delay in nanoseconds
  double delay(const Baseline baseline) const;

  /// @brief check whether a particular baseline has valid data
  /// @param[in] baseline baseline index
  /// @return true, if the data corresponding to the given baseline are valid
  bool isValid(const Baseline baseline) const;

  /// @brief obtain time
  /// @return UT epoch in days since 0 MJD
  double time() const;
 
  // monitoring information for the last correlated data
      
  /// @brief the beam this structure corresponds to [0..nBeam-1]
  int itsBeam;

  /// @brief averaged amplitudes for all 3 baselines in raw counts
  /// @details This vector is supposed to have 3 elements at all times. The order
  /// of baselines is 1-2,2-3,1-3 (1-based indices).
  std::vector<float> itsAmplitudes;
  
  /// @brief averaged phases for all 3 baselines in degrees
  /// @details This vector is supposed to have 3 elements at all times. The order
  /// of baselines is 1-2,2-3,1-3 (1-based indices).
  std::vector<float> itsPhases;
    
  /// @brief delays for all 3 baselines in nanoseconds
  /// @details This vector is supposed to have 3 elements at all times. The order
  /// of baselines is 1-2,2-3,1-3 (1-based indices).
  std::vector<double> itsDelays;
  
  /// @brief flagging information
  /// @details This vector is supposed to have 3 elements at all times. True for a 
  /// particular baseline means that the corresponding amplitudes, phases and delays
  /// are not valid. The order of baselines is 1-2,2-3,1-3 (1-based indices).
  std::vector<bool> itsFlags;
  
  /// @brief UT time in days since 0 MJD
  double itsTime;  
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_CORRINTERFACES_MONITORING_DATA_H

