/// @file FrtHWAndDrx.cc
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
#include "ingestpipeline/phasetracktask/FrtHWAndDrx.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include <casa/Arrays/Vector.h>

// std includes
#include <string>
#include <map>

ASKAP_LOGGER(logger, ".FrtHWAndDrx");

namespace askap {
namespace cp {
namespace ingest {

/// @brief Constructor.
/// @param[in] parset the configuration parameter set.
// @param[in] config configuration
FrtHWAndDrx::FrtHWAndDrx(const LOFAR::ParameterSet& parset, const Configuration& config) : 
       itsFrtComm(parset, config), 
       itsDRxDelayTolerance(static_cast<int>(parset.getUint32("drxdelaystep",0u))), 
       itsTm(config.antennas().size(),0.),
       itsPhases(config.antennas().size(),0.),
       itsUpdateTimeOffset(static_cast<int32_t>(parset.getInt32("updatetimeoffset")))
{
   if (itsDRxDelayTolerance == 0) {
       ASKAPLOG_INFO_STR(logger, "DRx delays will be updated every time the delay changes by 1.3 ns");
   } else {
       ASKAPLOG_INFO_STR(logger, "DRx delays will be updated when the required delay diverges more than "
               << itsDRxDelayTolerance << " 1.3ns steps");
   } 

   if (itsUpdateTimeOffset == 0) {
      ASKAPLOG_INFO_STR(logger, "The reported BAT of the fringe rotator parameter update will be used as is without any adjustment");
   } else {
      ASKAPLOG_INFO_STR(logger, "The reported BAT of the fringe rotator parameter update will be shifted by "<<itsUpdateTimeOffset<<" microseconds");
   }

   const std::vector<Antenna> antennas = config.antennas();
   const size_t nAnt = antennas.size();
   const casa::String refName = casa::downcase(parset.getString("refant"));
   itsRefAntIndex = nAnt;
   for (casa::uInt ant=0; ant<nAnt; ++ant) {
        if (casa::downcase(antennas.at(ant).name()) == refName) {
            itsRefAntIndex = ant;
            break;
        }
   }  
   ASKAPCHECK(itsRefAntIndex < nAnt, "Reference antenna "<<refName<<" is not found in the configuration");
   ASKAPLOG_INFO_STR(logger, "Will use "<<refName<<" (antenna index "<<itsRefAntIndex<<") as a reference antenna");
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
/// @param[in] rates matrix with phase rates for all antennas (rows) and 
///                  beams (columns) in radians per second
/// @param[in] effLO effective LO frequency in Hz
void FrtHWAndDrx::process(const askap::cp::common::VisChunk::ShPtr& chunk, 
              const casa::Matrix<double> &delays, const casa::Matrix<double> &rates, const double effLO)
{
  ASKAPDEBUGASSERT(delays.ncolumn() > 0);
  ASKAPDEBUGASSERT(itsRefAntIndex < delays.nrow());
  ASKAPDEBUGASSERT(delays.ncolumn() == rates.ncolumn());
  ASKAPDEBUGASSERT(delays.nrow() == rates.nrow());
  // signal about new timestamp (there is no much point to mess around with threads
  // as actions are tide down to correlator cycles
  itsFrtComm.newTimeStamp(chunk->time());

  const double samplePeriod = 1./768e6; // sample rate is 768 MHz
  // HW phase rate units are 2^{-28} turns per FFB sample of 54 microseconds
  const double phaseRateUnit = 2. * casa::C::pi / 268435456. / 54e-6;

  const double integrationTime = chunk->interval();
  ASKAPASSERT(integrationTime > 0);
  for (casa::uInt ant = 0; ant < delays.nrow(); ++ant) {
       // negate the sign here because we want to compensate the delay
       const double diffDelay = (delays(itsRefAntIndex,0) - delays(ant,0))/samplePeriod;
       // ideal delay
       ASKAPLOG_INFO_STR(logger, "delays between "<<ant<<" and ref="<<itsRefAntIndex<<" are "
               <<diffDelay*samplePeriod*1e9<<" ns");
       casa::Int drxDelay = static_cast<casa::Int>(2048. + diffDelay);
       if (drxDelay < 0) {
           ASKAPLOG_WARN_STR(logger, "DRx delay for antenna "<<ant<<" is out of range (below 0)");
           drxDelay = 0;
       }
       if (drxDelay > 4095) {
           ASKAPLOG_WARN_STR(logger, "DRx delay for antenna "<<ant<<" is out of range (exceeds 4095)");
           drxDelay = 4095;
       }
       // differential rate, negate the sign because we want to compensate here
       casa::Int diffRate = static_cast<casa::Int>((rates(itsRefAntIndex,0) - rates(ant,0))/phaseRateUnit);
       
       /*
       if (ant == 0) {
           const double interval = itsTm[ant]>0 ? (chunk->time().getTime("s").getValue() - itsTm[ant]) : 0;
           //diffRate = (int(interval/240) % 2 == 0 ? +1. : -1) * static_cast<casa::Int>(casa::C::pi / 100. / phaseRateUnit);
           const casa::Int rates[11] = {-10, -8, -6, -4, -2, 0, 2, 4, 6, 8,10}; 
           const double addRate = rates[int(interval/180) % 11];
           diffRate = addRate;
           if (int((interval - 5.)/180) % 11 != int(interval/180) % 11) {
               ASKAPLOG_DEBUG_STR(logger,"Invalidating ant="<<ant);
               itsFrtComm.invalidate(ant);
           }
           
           ASKAPLOG_DEBUG_STR(logger, "Interval = "<<interval<<" seconds, rate = "<<diffRate<<" for ant = "<<ant<<" addRate="<<addRate);
       }  else { diffRate = 0.;}
       if (itsTm[ant]<=0) {
           itsTm[ant] = chunk->time().getTime("s").getValue();
       }
       */
           
       if (diffRate > 131071) {
           ASKAPLOG_WARN_STR(logger, "Phase rate for antenna "<<ant<<" is outside the range (exeeds 131071)");
           diffRate = 131071;
       }
       if (diffRate < -131070) {
           ASKAPLOG_WARN_STR(logger, "Phase rate for antenna "<<ant<<" is outside the range (below -131070)");
           diffRate = -131070;
       }
       if ((abs(diffRate - itsFrtComm.requestedFRPhaseRate(ant)) > 20) || itsFrtComm.isUninitialised(ant)) {
          if ((abs(drxDelay - itsFrtComm.requestedDRxDelay(ant)) > itsDRxDelayTolerance) || itsFrtComm.isUninitialised(ant)) {
              ASKAPLOG_INFO_STR(logger, "Set DRx delays for antenna "<<ant<<" to "<<drxDelay<<" and phase rate to "<<diffRate);
              itsFrtComm.setDRxAndFRParameters(ant, drxDelay, diffRate,0,0);
          } else {
              ASKAPLOG_INFO_STR(logger, "Set phase rate for antenna "<<ant<<" to "<<diffRate);
              itsFrtComm.setFRParameters(ant,diffRate,0,0);
          }
          itsPhases[ant] = 0.;
       } else {
          if ((abs(drxDelay - itsFrtComm.requestedDRxDelay(ant)) > itsDRxDelayTolerance) || itsFrtComm.isUninitialised(ant)) {
              ASKAPLOG_INFO_STR(logger, "Set DRx delays for antenna "<<ant<<" to "<<drxDelay);
              itsFrtComm.setDRxDelay(ant, drxDelay);
          }
       }
       ASKAPDEBUGASSERT(ant < itsTm.size())
       /*
       if (itsTm[ant] > 0) {
           itsPhases[ant] += (chunk->time().getTime("s").getValue() - itsTm[ant]) * phaseRateUnit * itsFrtComm.requestedFRPhaseRate(ant);
       }
       */
       if (itsFrtComm.hadFRUpdate(ant)) {
           // 25000 microseconds is the offset before event trigger and the application of phase rates/accumulator reset (specified in the osl script)
           // on top of this we have a user defined fudge offset (see #5736)
           const int32_t triggerOffset = 25000 + itsUpdateTimeOffset;
           const uint64_t lastReportedFRUpdateBAT = itsFrtComm.lastFRUpdateBAT(ant);
           ASKAPCHECK(static_cast<int64_t>(lastReportedFRUpdateBAT) > static_cast<int64_t>(triggerOffset), "The FR trigger offset "<<triggerOffset<<
                  " microseconds is supposed to be small compared to BAT="<<lastReportedFRUpdateBAT<<", ant="<<ant);
           const uint64_t lastFRUpdateBAT = lastReportedFRUpdateBAT + triggerOffset;
           const uint64_t currentBAT = epoch2bat(casa::MEpoch(chunk->time(),casa::MEpoch::UTC));
           if (currentBAT > lastFRUpdateBAT) {
               const uint64_t elapsedTime = currentBAT - lastFRUpdateBAT; 
               const double etInCycles = double(elapsedTime + itsUpdateTimeOffset) / integrationTime / 1e6;
           
               ASKAPLOG_DEBUG_STR(logger, "Antenna "<<ant<<": elapsed time since last FR update "<<double(elapsedTime)/1e6<<" s ("<<etInCycles<<" cycles)");
       
               itsPhases[ant] = double(elapsedTime) * 1e-6 * phaseRateUnit * itsFrtComm.requestedFRPhaseRate(ant);
           } else {
              ASKAPLOG_DEBUG_STR(logger, "Still processing old data before FR update event trigger for antenna "<<ant);
           }
       } // if FR had an update for a given antenna
  } // loop over antennas
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

           // attempt to correct for residual delays in software
           const casa::uInt beam1 = chunk->beam1()[row];
           const casa::uInt beam2 = chunk->beam2()[row];
           ASKAPDEBUGASSERT(beam1 < delays.ncolumn());
           ASKAPDEBUGASSERT(beam2 < delays.ncolumn());
           // actual delay, note the sign is flipped because we're correcting the delay here
           const double thisRowDelay = delays(ant1,beam1) - delays(ant2,beam2);
           const double residualDelay = thisRowDelay - appliedDelay;

           // actual rate
           //const double thisRowRate = rates(ant1,beam1) - rates(ant2,beam2);
              
           
           const double phaseDueToAppliedDelay = 2. * casa::C::pi * effLO * appliedDelay;
           const double phaseDueToAppliedRate = itsPhases[ant1] - itsPhases[ant2];
           const casa::Vector<casa::Double>& freq = chunk->frequency();
           ASKAPDEBUGASSERT(freq.nelements() == thisRow.nrow());
           for (casa::uInt chan = 0; chan < thisRow.nrow(); ++chan) {
                //casa::Vector<casa::Complex> thisChan = thisRow.row(chan);
                const float phase = static_cast<float>(phaseDueToAppliedDelay + phaseDueToAppliedRate +
                             2. * casa::C::pi * freq[chan] * residualDelay);
                const casa::Complex phasor(cos(phase), sin(phase));

                // actual rotation (same for all polarisations)
                for (casa::uInt pol = 0; pol < thisRow.ncolumn(); ++pol) {
                     thisRow(chan,pol) *= phasor;
                }
                //thisChan *= phasor;
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
