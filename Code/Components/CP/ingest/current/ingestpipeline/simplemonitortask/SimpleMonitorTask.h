/// @file
///
/// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_CP_INGEST_SIMPLEMONITORTASK_H
#define ASKAP_CP_INGEST_SIMPLEMONITORTASK_H

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"
#include "utils/DelayEstimator.h"

// casa includes
#include <casa/Arrays/Matrix.h>


// Local package includes
#include "ingestpipeline/ITask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too
#include "configuration/BaselineMap.h"

// std includes
#include <fstream>
#include <string>

namespace askap {
namespace cp {
namespace ingest {

/// @brief task for monitoring average data properties
/// @details This task is intended to be used in early commissioning experiments. It is 
/// an alternative diagnostics to check the average amplitude, phase and delay for the subset of
/// data managed by this particular rank (in the way similar to software correlator). This class is
/// not intended to survive in its current form in the long term.
class SimpleMonitorTask : public askap::cp::ingest::ITask {
public:

   /// @breif Constructor
   /// @param[in] parset the configuration parameter set.
   /// @param[in] config configuration
   SimpleMonitorTask(const LOFAR::ParameterSet& parset,
             const Configuration& config);

   /// @brief destructor
   ~SimpleMonitorTask();

   /// @brief Extract required information from visibilities in the specified VisChunk.
   /// @details There is no modification of the data, just internal buffers are updated.
   ///
   /// @param[in,out] chunk  the instance of VisChunk for which the
   ///                       phase factors will be applied.
   virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

protected:   
   /// @details Process one row of data.
   /// @param[in] vis vis spectrum for the given baseline/pol index to work with
   /// @param[in] baseline baseline ID 
   /// @param[in] beam beam ID
   void processRow(const casa::Vector<casa::Complex> &vis, const casa::uInt baseline, const casa::uInt beam);

   /// @brief Publish the buffer
   void publishBuffer();

private:
   /// @brief time corresponding to the active buffer
   double itsCurrentTime;

   /// @brief time of the first data point (or a negative value upon initialisation)
   double itsStartTime;

   /// @brief buffer for averaged visibility for each baseline/polarisation index and beam
   casa::Matrix<casa::Complex> itsVisBuffer;

   /// @brief buffer for delay for each baseline/polarisation index and beam
   casa::Matrix<casa::Double> itsDelayBuffer;

   /// @brief baselines/polarisation indices to monitor
   /// @details One can setup a subset of baselines to monitor. In particular, the current form of
   /// monitoring is not very suitable for cross-pols. The mapping is setup the same way as for the 
   /// main baseline map, but parset prefixes are different, for example:
   ///
   /// tasks.SimpleMonitor.params.baselineids = [0]
   /// tasks.SimpleMonitor.params.0 = [1,2,XX]
   ///
   BaselineMap itsBaselineMap;

   /// @brief delay estimator
   scimath::DelayEstimator itsDelayEstimator;

   /// @brief output file stream
   std::ofstream itsOStream;

   /// @brief output file name
   std::string itsFileName;
}; // PhaseTrackTask class

} // ingest

} // cp

} // askap

#endif // #ifndef ASKAP_CP_INGEST_SIMPLEMONITORTASK_H

