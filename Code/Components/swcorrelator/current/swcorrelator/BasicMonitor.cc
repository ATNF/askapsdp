/// @file 
///
/// @brief basic on-the-fly monitor dumping data into an ascii file
/// @details This implementation of the data monitor dumps delay and
/// visibility history into ascii files for on-the-fly monitoring along
/// with the latest spectra for each beam
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

#include <askap_swcorrelator.h>
#include <swcorrelator/BasicMonitor.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>
#include <utils/DelayEstimator.h>


#include <fstream>


ASKAP_LOGGER(logger, ".basicmonitor");

namespace askap {

namespace swcorrelator {

/// @brief constructor
BasicMonitor::BasicMonitor() : itsLastHistPosition(-1), itsWrapped(false) {}


/// @brief create and configure the monitor   
/// @details
/// @return shared pointer to the monitor
boost::shared_ptr<IMonitor> BasicMonitor::setup(const LOFAR::ParameterSet &) 
{
   ASKAPLOG_INFO_STR(logger, "Setting up Basic Data Monitor");  
   boost::shared_ptr<BasicMonitor> result(new BasicMonitor);
   return result;
}

/// @brief initialise publishing
/// @details Technically, this step is not required. But given the
/// current design of the code it seems better to give a hint on the maximum
/// possible number of antennas, beams and channels, e.g. to initialise caches.
/// @param[in] nAnt maximum number of antennas
/// @param[in] nBeam maximum number of beams
/// @param[in] nChan maximum number of channels
/// @note At the moment we envisage that this method would only be called once.
/// Technically all this information could be extracted from the parset supplied
/// in the setup method, but it seems handy to have each parameter extracted from
/// the parset at a single place only.  
void BasicMonitor::initialise(const int nAnt, const int nBeam, const int nChan)
{
  if (itsHistory.nelements() == 0) {
      const int nHistory = 620;
      ASKAPDEBUGASSERT(nAnt > 1);
      ASKAPDEBUGASSERT(nBeam > 0);
      ASKAPDEBUGASSERT(nChan > 0);
      const int nBaselines = nAnt * (nAnt - 1) / 2;
      itsLastHistPosition = -1;
      itsWrapped = false;
      itsHistory.resize(nHistory,nBeam,nBaselines);
      itsHistory.set(casa::Complex(0.,0.));
      itsDelayHistory.resize(nHistory, nBeam, nBaselines);
      itsDelayHistory.set(0.);
      itsControlHistory.resize(nHistory, nBeam, nBaselines);
      itsControlHistory.set(0u);
      itsBATs.resize(nHistory);
      itsBATs.set(0);
  }
  ASKAPDEBUGASSERT(itsHistory.shape() == itsDelayHistory.shape());
  ASKAPDEBUGASSERT(itsHistory.nrow() == itsBATs.nelements());
}

/// @brief advance history if necessary
/// @details Advances the cursor in the history list if the new bat is different from the one 
/// stored during the previous step (unless it is a first step)
/// @param[in] bat new BAT  
void BasicMonitor::advanceHistoryCursor(const uint64_t bat)
{
  if (itsLastHistPosition >= 0) {
      if (bat == itsBATs[itsLastHistPosition]) {
          return;
      } else if (bat < itsBATs[itsLastHistPosition]) {
          ASKAPLOG_DEBUG_STR(logger, "New BAT = "<<bat<<" is earlier than the last history item BAT="<<
                 itsBATs[itsLastHistPosition]);
      }
  }
  ++itsLastHistPosition;
  if (itsLastHistPosition >= int(itsHistory.nrow())) {
      itsLastHistPosition = 0;
      itsWrapped = true;
  }  
  ASKAPDEBUGASSERT(itsLastHistPosition >= 0);
  ASKAPDEBUGASSERT(itsLastHistPosition < int(itsBATs.nelements()));
  itsBATs[itsLastHistPosition] = bat;
}
   
/// @brief Publish one buffer of data
/// @details This method is called as soon as the new chunk of data is written out
/// @param[in] buf products buffer
/// @note the buffer is locked for the duration of execution of this method, different
/// beams are published separately
void BasicMonitor::publish(const CorrProducts &buf) 
{
  advanceHistoryCursor(buf.itsBAT);
  {
    const std::string fname = "spc_beam" + utility::toString<int>(buf.itsBeam) + ".dat";
    std::ofstream os(fname.c_str());
    for (casa::uInt chan=0; chan < buf.itsVisibility.ncolumn(); ++chan) {
         os<<chan<<" ";
         for (casa::uInt baseline = 0; baseline < buf.itsVisibility.nrow(); ++baseline) {
              os<<abs(buf.itsVisibility(baseline,chan))<<" "<<arg(buf.itsVisibility(baseline,chan))/casa::C::pi*180.<<" ";                       
         }
         os<<std::endl;
    }
  }
  
  const casa::Vector<casa::Float> delays = estimateDelays(buf.itsVisibility);
  ASKAPLOG_DEBUG_STR(logger, "Beam "<<buf.itsBeam<<": delays (s) = "<<delays);
  ASKAPDEBUGASSERT(delays.nelements() == buf.itsVisibility.nrow());
  if (buf.itsBeam >= int(itsHistory.ncolumn())) {
      ASKAPLOG_DEBUG_STR(logger, "Receive buffer corresponding to beam "<<buf.itsBeam<<
           " which exceeds the maximum number of beams "<<itsHistory.ncolumn());
      return;     
  }
            
  for (casa::uInt baseline = 0; baseline < buf.itsVisibility.nrow(); ++baseline) {
                 
       itsDelayHistory(itsLastHistPosition, buf.itsBeam, baseline) = delays[baseline];
                 
       // control is actually per antenna, but number of antennas is equal to the number of baselines
       itsControlHistory(itsLastHistPosition, buf.itsBeam, baseline) = buf.itsControl[baseline];         
       
       // average in frequency
       casa::Complex temp(0.,0.);
       for (casa::uInt chan=0; chan < buf.itsVisibility.ncolumn(); ++chan) {
            temp += buf.itsVisibility(baseline,chan);
       }
       itsHistory(itsLastHistPosition,buf.itsBeam,baseline) = temp / float(buf.itsVisibility.ncolumn());
              
       /*
       // middle of the band
       const casa::uInt chan = buf.itsVisibility.ncolumn() / 2;
       ASKAPDEBUGASSERT(chan < buf.itsVisibility.ncolumn());
       itsHistory(itsLastHistPosition,buf.itsBeam,baseline) = buf.itsVisibility(baseline,chan);
       */
  }
  
}
  
/// @brief finilaise publishing for the current integration
/// @details This method is called when data corresponding to all beams are published.
/// It is the place for operations which do not require the lock on the buffers
/// (i.e. dumping the accumulated history to the file, etc).
void BasicMonitor::finalise()
{
  // dump the time history to a file
  std::ofstream os("visplot.dat");
  const int nHistory = int(itsBATs.nelements());
  for (int i=0, curPos = itsWrapped ? itsLastHistPosition + 1 : 0; i<nHistory; ++i,++curPos) {
      if (curPos >= nHistory) {
          curPos = 0;
          if (!itsWrapped) {
              break;
          }
      }
      os<<itsBATs[curPos]<<" ";
      for (int beam=0; beam < int(itsHistory.ncolumn()); ++beam) {
           for (casa::uInt baseline = 0; baseline < itsHistory.nplane(); ++baseline) {
                os<<abs(itsHistory(curPos,beam,baseline))<<" "<<arg(itsHistory(curPos,beam,baseline))/casa::C::pi*180.<<" "
                  <<itsDelayHistory(curPos,beam,baseline)*1e9<<" ";
           }
      }
      // only show control field for the first beam (it should be the same)
      for (casa::uInt baseline = 0; baseline < itsHistory.nplane(); ++baseline) {
           os<<itsControlHistory(curPos,0,baseline)<<" ";
      }
      //
      os<<std::endl;
      if (curPos == itsLastHistPosition) {
          break;
      }
  }  
}

/// @brief helper method to get delays
/// @details
/// @param[in] vis visibility matrix (rows are baselines, columns are channels)
/// @return delays in seconds for each baseline
/// @note the routine assumes 1 MHz channel spacing and will not work for a very quick wrap
casa::Vector<casa::Float> BasicMonitor::estimateDelays(const casa::Matrix<casa::Complex> &vis)
{
  casa::Vector<casa::Float> result(vis.nrow(),0.);
  if (vis.ncolumn() >= 2) {
      scimath::DelayEstimator de(1e6); // hard-coded 1 MHz resolution
      for (casa::uInt row = 0; row < vis.nrow(); ++row) {
           result[row] = float(de.getDelay(vis.row(row)));
      }
  }    
  return result;  
}

} // namespace swcorrelator

} // namespace askap

