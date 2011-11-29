/// @file 
///
/// @brief Actual MS writer class doing the low-level dirty job
/// @details This class is heavily based on Ben's MSSink in the CP/ingestpipeline package.
/// I just copied the appropriate code from there. The basic approach is to set up as
/// much of the metadata as we can via the parset file. It is envisaged that we may
/// use this class also for the conversion of the DiFX output into MS. 
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


#ifndef ASKAP_SWCORRELATOR_FILLER_MS_SINK
#define ASKAP_SWCORRELATOR_FILLER_MS_SINK

// casa includes
#include "ms/MeasurementSets/MeasurementSet.h"

// other 3rd party
#include <Common/ParameterSet.h>

// boost includes
#include "boost/scoped_ptr.hpp"

namespace askap {

namespace swcorrelator {

/// @brief Actual MS writer class doing the low-level dirty job
/// @details This class is heavily based on Ben's MSSink in the CP/ingestpipeline package.
/// I just copied the appropriate code from there. The basic approach is to set up as
/// much of the metadata as we can via the parset file. It is envisaged that we may
/// use this class also for the conversion of the DiFX output into MS. 
/// @ingroup swcorrelator
class FillerMSSink {
public:
  /// @brief constructor, sets up  MS writer
  /// @details Configuration is done via the parset, a lot of the metadata are just filled
  /// via the parset.
  /// @param[in] parset parset file with configuration info
  FillerMSSink(const LOFAR::ParameterSet &parset);

protected:
  /// @brief Initialises the ANTENNA table
  /// @details This method extracts configuration from the parset and fills in the 
  /// compulsory ANTENNA table. It also caches antenna positions in the form suitable for
  /// calculation of uvw's.
  void initAntennas();
  
  /// @brief Initialises the FEED table
  /// @details This method reads in feed information given in the parset and writes a dummy 
  /// feed table
  void initFeeds();
  
  /// @brief Initialises the OBSERVATION table
  /// @details This method sets up observation table and fills some dummy data from the parset
  void initObs();

  /// @brief Create the measurement set
  void create();
  
private:
  /// @brief parameters
  LOFAR::ParameterSet itsParset;
  
  /// @brief Measurement set
  boost::scoped_ptr<casa::MeasurementSet> itsMs;
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_FILLER_MS_SINK


