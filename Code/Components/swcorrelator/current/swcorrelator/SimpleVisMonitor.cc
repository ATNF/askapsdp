/// @file 
///
/// @brief basic on-the-fly monitor dumping data into an ascii file
/// @details This implementation of the data monitor dumps delay and
/// visibility history into ascii files for on-the-fly monitoring along
/// with the latest spectra for each beam. Unlike BasicMonitor, this
/// one opens an ascii file in the constructor and appends the data
/// indefinitely.
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
#include <swcorrelator/SimpleVisMonitor.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>


#include <fstream>


ASKAP_LOGGER(logger, ".simplevismonitor");

namespace askap {

namespace swcorrelator {

/// @brief constructor
SimpleVisMonitor::SimpleVisMonitor() : itsStartBAT(0ul), itsBAT(0ul) {}


/// @brief create and configure the monitor   
/// @details
/// @return shared pointer to the monitor
boost::shared_ptr<IMonitor> SimpleVisMonitor::setup(const LOFAR::ParameterSet &) 
{
   ASKAPLOG_INFO_STR(logger, "Setting up Simple Visibility Data Monitor");  
   boost::shared_ptr<SimpleVisMonitor> result(new SimpleVisMonitor);
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
void SimpleVisMonitor::initialise(const int nAnt, const int nBeam, const int nChan)
{
  ASKAPASSERT(nAnt > 1);
  ASKAPASSERT(nBeam > 0);
  ASKAPASSERT(nChan > 0);
  const int nBaselines = nAnt * (nAnt - 1) /2;
  itsVisBuffer.resize(nBeam,nBaselines);
  itsDelayBuffer.resize(nBeam,nBaselines);
  itsControlBuffer.resize(nAnt);
  itsVisBuffer.set(casa::Complex(0.,0.));
  itsDelayBuffer.set(0.);
  itsControlBuffer.set(0u);
}

/// @brief Publish one buffer of data
/// @details This method is called as soon as the new chunk of data is written out
/// @param[in] buf products buffer
/// @note the buffer is locked for the duration of execution of this method, different
/// beams are published separately
void SimpleVisMonitor::publish(const CorrProducts &buf) 
{
  if (itsStartBAT == 0ul) {
      itsStartBAT = buf.itsBAT;
  }
  if (itsBAT != buf.itsBAT) {
      itsBAT = buf.itsBAT;
      itsVisBuffer.set(casa::Complex(0.,0.));
      itsDelayBuffer.set(0.);
      itsControlBuffer.set(0u);
  }
     
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
  
  const casa::Vector<casa::Float> delays = BasicMonitor::estimateDelays(buf.itsVisibility);
  ASKAPLOG_DEBUG_STR(logger, "Beam "<<buf.itsBeam<<": delays (s) = "<<delays);
  ASKAPDEBUGASSERT(delays.nelements() == buf.itsVisibility.nrow());
  if (buf.itsBeam >= int(itsVisBuffer.nrow())) {
      ASKAPLOG_DEBUG_STR(logger, "Receive buffer corresponding to beam "<<buf.itsBeam<<
           " which exceeds the maximum number of beams "<<int(itsVisBuffer.nrow()));
      return;     
  }
            
  for (casa::uInt baseline = 0; baseline < buf.itsVisibility.nrow(); ++baseline) {
                 
       itsDelayBuffer(buf.itsBeam, baseline) = delays[baseline];
                 
       // control is actually per antenna, but number of antennas is equal to the number of baselines
       // and we capture the first beam only
       if (buf.itsBeam == 0) {
           itsControlBuffer[baseline] = buf.itsControl[baseline];         
       }
       
       // average in frequency
       casa::Complex temp(0.,0.);
       for (casa::uInt chan=0; chan < buf.itsVisibility.ncolumn(); ++chan) {
            temp += buf.itsVisibility(baseline,chan);
       }
       itsVisBuffer(buf.itsBeam,baseline) = temp / float(buf.itsVisibility.ncolumn());
  }
  
}
  
/// @brief finilaise publishing for the current integration
/// @details This method is called when data corresponding to all beams are published.
/// It is the place for operations which do not require the lock on the buffers
/// (i.e. dumping the accumulated history to the file, etc).
void SimpleVisMonitor::finalise()
{
  if (!itsOStream.is_open()) {
      try {
         itsOStream.open("visplot.dat");
      }
      catch (const std::exception &ex) {
         ASKAPLOG_FATAL_STR(logger, "Error opening output ascii file for monitoring information: "<<ex.what());
      }
  }
  itsOStream<<double(itsBAT-itsStartBAT)/1e6/60.<<" ";
  for (int beam=0; beam < int(itsVisBuffer.nrow()); ++beam) {
       for (casa::uInt baseline = 0; baseline < itsVisBuffer.ncolumn(); ++baseline) {
            itsOStream<<abs(itsVisBuffer(beam,baseline))<<" "<<arg(itsVisBuffer(beam,baseline))/casa::C::pi*180.<<" "
               <<itsDelayBuffer(beam,baseline)*1e9<<" ";
        }
  }
  // only show control field for the first beam (it should be the same)
  for (casa::uInt baseline = 0; baseline < itsControlBuffer.nelements(); ++baseline) {
       itsOStream<<itsControlBuffer[baseline]<<" ";
  }
  //
  itsOStream<<std::endl;
  itsOStream.flush();
}

} // namespace swcorrelator

} // namespace askap

