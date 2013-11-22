/// @file FrtDrxDelays.cc
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

// Local package includes
#include "ingestpipeline/phasetracktask/FrtDrxDelays.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"

// std includes
#include <string>
#include <map>

ASKAP_LOGGER(logger, ".PhaseTrackTask");

namespace askap {
namespace cp {
namespace ingest {

// simplest fringe rotation method, essentially just a proof of concept

/// @brief Constructor.
/// @param[in] parset the configuration parameter set.
// @param[in] config configuration
FrtDrxDelays::FrtDrxDelays(const LOFAR::ParameterSet& parset, const Configuration& /*config*/)
{
   const std::string locatorHost = parset.getString("ice.locator_host");
   const std::string locatorPort = parset.getString("ice.locator_port");
   const std::string topicManager = parset.getString("icestorm.topicmanager");
   const std::string outtopic = parset.getString("icestorm.outtopic");
   const std::string intopic = parset.getString("icestorm.intopic");
   const std::string adapterName = "FrtDrxDelays";
   const int bufSize = 24;
   ASKAPLOG_INFO_STR(logger, "Creating simple delay/phase tracker working via DRx, ice topics: "<<outtopic<<" and "<<intopic);

   itsOutPort.reset(new icewrapper::FrtMetadataOutputPort(locatorHost, locatorPort, topicManager, outtopic));
   itsInPort.reset(new FrtMetadataSource(locatorHost, locatorPort, topicManager, intopic, adapterName, bufSize));
   ASKAPDEBUGASSERT(itsOutPort);
   ASKAPDEBUGASSERT(itsInPort);
}

/// Process a VisChunk.
///
/// This method is called once for each correlator integration.
/// 
/// @param[in] chunk    a shared pointer to a VisChunk object. The
///             VisChunk contains all the visibilities and associated
///             metadata for a single correlator integration. This method
///             is expected to correct visibilities in this VisChunk 
///             as required (some methods may not need to do any correction at all)
/// @param[in] delays matrix with delays for all antennas (rows) and beams (columns) in seconds
// @param[in] rates matrix with phase rates for all antennas (rows) and 
//                  beams (columns) in radians per second
/// @param[in] effLO effective LO frequency in Hz
void FrtDrxDelays::process(const askap::cp::common::VisChunk::ShPtr& chunk, 
              const casa::Matrix<double> &delays, const casa::Matrix<double> &/*rates*/, const double effLO)
{
  ASKAPDEBUGASSERT(delays.ncolumn() > 0);

  if (itsDRxDelays.nelements() < delays.nrow()) {
      itsDRxDelays.resize(delays.nrow());
      itsDRxDelays.set(4097); // outside the range and it wouldn't match
  }
  std::map<std::string, int> msg;
  const double samplePeriod = 1./768e6; // sample rate is 768 MHz
  for (casa::uInt ant = 0; ant < delays.nrow(); ++ant) {
       const double diffDelay = (delays(ant,0) - delays(0,0))/samplePeriod;
       casa::Int drxDelay = static_cast<casa::Int>(2048. + diffDelay);
       if (drxDelay < 0) {
           ASKAPLOG_WARN_STR(logger, "DRx delay for antenna "<<ant<<" is out of range (below 0)");
           drxDelay = 0;
       }
       if (drxDelay > 4095) {
           ASKAPLOG_WARN_STR(logger, "DRx delay for antenna "<<ant<<" is out of range (exceeds 4095)");
           drxDelay = 4095;
       }
       const casa::uInt uDRxDelay = static_cast<casa::uInt>(drxDelay);
       if (uDRxDelay != itsDRxDelays[ant]) { 
           // add delay request to the message (need to sort out key names and antenna naming later)
           msg["ant"+utility::toString(ant)] = uDRxDelay;
           itsDRxDelays[ant] = uDRxDelay;
           ASKAPLOG_INFO_STR(logger, "Set DRx delays for antenna "<<ant<<" to "<<drxDelay);
       }
  }
  if (msg.size()) {
      // send the message
      itsOutPort->send(msg);
  }
  // we don't really need the reply at this stage, just pop it out of the queue for a test
  boost::shared_ptr<std::map<std::string,int> > reply = itsInPort->next(0);
  if (reply) {
      ASKAPLOG_INFO_STR(logger, "Received a frt reply message");
  }
  //
  for (casa::uInt row = 0; row < chunk->nRow(); ++row) {
       // slice to get this row of data
       casa::Matrix<casa::Complex> thisRow = chunk->visibility().yzPlane(row);
       const casa::uInt ant1 = chunk->antenna1()[row];
       const casa::uInt ant2 = chunk->antenna2()[row];
       ASKAPDEBUGASSERT(ant1 < delays.nrow());
       ASKAPDEBUGASSERT(ant2 < delays.nrow());
       const double thisRowDelay = delays(ant2,0) - delays(ant1,0);

       const float phase = -2. * static_cast<float>(casa::C::pi * effLO * thisRowDelay);
       const casa::Complex phasor(cos(phase), sin(phase));

       // actual rotation
       thisRow *= phasor;
  }
}

} // namespace ingest 
} // namespace cp 
} // namespace askap 

