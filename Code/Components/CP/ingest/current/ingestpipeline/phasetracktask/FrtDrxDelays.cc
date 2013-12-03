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

ASKAP_LOGGER(logger, ".FrtDrxDelay");

namespace askap {
namespace cp {
namespace ingest {

// simplest fringe rotation method, essentially just a proof of concept

/// @brief Constructor.
/// @param[in] parset the configuration parameter set.
// @param[in] config configuration
FrtDrxDelays::FrtDrxDelays(const LOFAR::ParameterSet& parset, const Configuration& config) : 
       itsFrtComm(parset, config), 
       itsDRxDelayTolerance(static_cast<int>(parset.getUint32("drxdelaystep",0u))),
       itsTrackResidualDelay(parset.getBool("trackresidual",true))
{
   if (itsDRxDelayTolerance == 0) {
       ASKAPLOG_INFO_STR(logger, "DRx delays will be updated every time the delay changes by 1.3 ns");
   } else {
       ASKAPLOG_INFO_STR(logger, "DRx delays will be updated when the required delay diverges more than "<<itsDRxDelayTolerance<<
                         " 1.3ns steps");
   } 
   if (itsTrackResidualDelay) {
       ASKAPLOG_INFO_STR(logger, "Residual delays and phases will be tracked in software");       
   } else {
       ASKAPLOG_INFO_STR(logger, "No attempt to track the residual delays and phases in software will be made");       
   }
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
  // signal about new timestamp (there is no much point to mess around with threads as actions are tide down to correlator cycles
  itsFrtComm.newTimeStamp(chunk->time());

  const double samplePeriod = 1./768e6; // sample rate is 768 MHz
  for (casa::uInt ant = 0; ant < delays.nrow(); ++ant) {
       const double diffDelay = (delays(ant,0) - delays(0,0))/samplePeriod;
       // ideal delay
       casa::Int drxDelay = static_cast<casa::Int>(2048. + diffDelay);
       if (drxDelay < 0) {
           ASKAPLOG_WARN_STR(logger, "DRx delay for antenna "<<ant<<" is out of range (below 0)");
           drxDelay = 0;
       }
       if (drxDelay > 4095) {
           ASKAPLOG_WARN_STR(logger, "DRx delay for antenna "<<ant<<" is out of range (exceeds 4095)");
           drxDelay = 4095;
       }
       if ((abs(drxDelay - itsFrtComm.requestedDRxDelay(ant)) < itsDRxDelayTolerance) || itsFrtComm.isUninitialised(ant)) {
           ASKAPLOG_INFO_STR(logger, "Set DRx delays for antenna "<<ant<<" to "<<drxDelay);
           itsFrtComm.setDRxDelay(ant, drxDelay);
       }
  }
  //
  for (casa::uInt row = 0; row < chunk->nRow(); ++row) {
       // slice to get this row of data
       const casa::uInt ant1 = chunk->antenna1()[row];
       const casa::uInt ant2 = chunk->antenna2()[row];
       ASKAPDEBUGASSERT(ant1 < delays.nrow());
       ASKAPDEBUGASSERT(ant2 < delays.nrow());
       if (itsFrtComm.isValid(ant1) && itsFrtComm.isValid(ant2)) {
           // desired delays are set and applied, do phase rotation
           casa::Matrix<casa::Complex> thisRow = chunk->visibility().yzPlane(row);
           const double appliedDelay = samplePeriod * (itsFrtComm.requestedDRxDelay(ant2)-itsFrtComm.requestedDRxDelay(ant1));

           if (itsTrackResidualDelay) {
               // attempt to correct for residual delays in software
               const casa::uInt beam1 = chunk->beam1()[row];
               const casa::uInt beam2 = chunk->beam2()[row];
               ASKAPDEBUGASSERT(beam1 < delays.ncolumn());
               ASKAPDEBUGASSERT(beam2 < delays.ncolumn());
               // actual delay
               const double thisRowDelay = delays(ant2,beam2) - delays(ant1,beam1);
               const double residualDelay = thisRowDelay - appliedDelay;
               
               const double phaseDueToAppliedDelay = -2. * casa::C::pi * effLO * appliedDelay;
               const casa::Vector<casa::Double>& freq = chunk->frequency();
               ASKAPDEBUGASSERT(freq.nelements() == thisRow.nrow());
               for (casa::uInt chan = 0; chan < thisRow.nrow(); ++chan) {
                    casa::Vector<casa::Complex> thisChan = thisRow.row(chan);
                    const float phase = static_cast<float>(phaseDueToAppliedDelay - 
                                 2. * casa::C::pi * freq[chan] * residualDelay);
                    const casa::Complex phasor(cos(phase), sin(phase));

                    // actual rotation (same for all polarisations)
                    thisChan *= phasor;
               }
               
           } else {
              // just correct phases corresponding to the applied delay in IF (simple phase tracking) 
              const float phase = -2. * static_cast<float>(casa::C::pi * effLO * appliedDelay);
              const casa::Complex phasor(cos(phase), sin(phase));

              // actual rotation
              thisRow *= phasor;
           }
       } else {
         // the parameters for these antennas are being changed, flag the data
         casa::Matrix<casa::Bool> thisFlagRow = chunk->flag().yzPlane(row);
         thisFlagRow.set(casa::True); 
       }
  }
}

} // namespace ingest 
} // namespace cp 
} // namespace askap 

