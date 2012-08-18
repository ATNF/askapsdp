/// @file 
///
/// @brief Writing thread of the MS filler
/// @details This class holds a shared pointer to the main filler and can call
/// its methods to get data and to synchronise.
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

#include <swcorrelator/FillerWorker.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>

ASKAP_LOGGER(logger, ".corrfiller");

#include <string>
#include <fstream>
#include <vector>
#include <boost/thread.hpp>

namespace askap {

namespace swcorrelator {


/// @brief helper method to get delays (could be made a protected member of the class)
/// @details
/// @param[in] vis visibility matrix (rows are baselines, columns are channels)
/// @return delays in seconds for each baseline
/// @note the routine assumes 1 MHz channel spacing and will not work for a very quick wrap
casa::Vector<casa::Float> estimateDelays(const casa::Matrix<casa::Complex> &vis) 
{
  casa::Vector<casa::Float> result(vis.nrow(),0.);
  if (vis.ncolumn() >= 2) {
      std::vector<float> phases(vis.ncolumn());
      const float threshold = 3 * casa::C::pi / 2;
      for (casa::uInt row = 0; row < vis.nrow(); ++row) {
           // unambiguate phases
           float wrapCompensation = 0.;
           for (size_t chan=0; chan<phases.size(); ++chan) {
                const casa::Float curPhase = arg(vis(row,casa::uInt(chan)));
                if (chan > 0) {
                    const float prevOrigPhase = phases[chan - 1] - wrapCompensation;
                    const float diff = curPhase - prevOrigPhase;
                    if (diff >= threshold) {
                        wrapCompensation -= 2. * casa::C::pi;
                    } else if (diff <= -threshold) {
                        wrapCompensation += 2. * casa::C::pi;
                    }
                }
                phases[chan] = curPhase + wrapCompensation;
           }
           /*
           // for debugging
           if (row == 0) {
               std::ofstream os("phtest.dat");
               for (size_t chan=0; chan<phases.size(); ++chan) {
                   os<<chan<<" "<<phases[chan] / casa::C::pi * 180. <<" "<<arg(vis(row,casa::uInt(chan))) / casa::C::pi * 180.<< std::endl;
               }
           }
           //
           */
           // do LSF into phase vs. channel
           double sx = 0., sy = 0., sx2 = 0., sxy = 0.;
           // could've combined two loops, but keep it easy for now
           for (size_t chan=0; chan < phases.size(); ++chan) {
                sx += double(chan);
                sx2 += double(chan)*double(chan);
                sy += double(phases[chan]);
                sxy += double(chan)*double(phases[chan]);
           }
           sx /= double(phases.size());
           sy /= double(phases.size());
           sx2 /= double(phases.size());
           sxy /= double(phases.size());
           const double coeff = (sxy - sx * sy) / (sx2 - sx * sx);
           result[row] = float(coeff / 2. / casa::C::pi / 1e6);
      }
  }    
  return result;
}

/// @brief constructor, pass the shared pointer to the filler
FillerWorker::FillerWorker(const boost::shared_ptr<CorrFiller> &filler) : itsFiller(filler) 
{
  ASKAPDEBUGASSERT(itsFiller);
}

/// @brief entry point for the parallel thread
void FillerWorker::operator()()
{
  ASKAPLOG_INFO_STR(logger, "Writing thread started, id="<<boost::this_thread::get_id());
  try {
    ASKAPDEBUGASSERT(itsFiller);
    const int nHistory = 620;
    int lastHistPos = -1;
    bool wasWrapped = false;
    casa::Cube<casa::Complex> history(nHistory,itsFiller->nBeam(),3,casa::Complex(0.,0.));
    casa::Cube<casa::Float> delayHistory(nHistory, itsFiller->nBeam(),3, 0.);
    std::vector<uint64_t> bats(nHistory,0);
    
    while (true) {       
       const bool buffer = itsFiller->getWritingJob();
       const std::string bufType = buffer ? "first" : "second";
       // for history
       ++lastHistPos;
       if (lastHistPos >= int(history.nrow())) {
           lastHistPos = 0;
           wasWrapped = true;
       }
       //
       for (int beam=0; beam < itsFiller->nBeam(); ++beam) {
            CorrProducts &cp = itsFiller->getProductsToWrite(beam, buffer);
            ASKAPLOG_INFO_STR(logger, "Write for buffer `"<<bufType<<"` beam="<<beam<<" bat="<<cp.itsBAT<<
               " vis="<<cp.itsVisibility<<" flag="<<cp.itsFlag);
            
            // write data into MS
            itsFiller->resultSink().write(cp);
               
            // for real-time monitoring
            if (beam == 0) {
                bats[lastHistPos] = cp.itsBAT;                
            }
            {
                const std::string fname = "spc_beam" + utility::toString<int>(beam) + ".dat";
                std::ofstream os(fname.c_str());
                for (casa::uInt chan=0; chan < cp.itsVisibility.ncolumn(); ++chan) {
                    os<<chan<<" ";
                    for (casa::uInt baseline = 0; baseline < cp.itsVisibility.nrow(); ++baseline) {
                         os<<abs(cp.itsVisibility(baseline,chan))<<" "<<arg(cp.itsVisibility(baseline,chan))/casa::C::pi*180.<<" ";                       
                    }
                    os<<std::endl;
                }
            }
            const casa::Vector<casa::Float> delays = estimateDelays(cp.itsVisibility);
            ASKAPLOG_DEBUG_STR(logger, "Beam "<<beam<<": delays (s) = "<<delays);
            ASKAPDEBUGASSERT(delays.nelements() == cp.itsVisibility.nrow());
            
            for (casa::uInt baseline = 0; baseline < cp.itsVisibility.nrow(); ++baseline) {
                 
                 delayHistory(lastHistPos, beam, baseline) = delays[baseline];
                 
                 
                 // average in frequency
                 casa::Complex temp(0.,0.);
                 for (casa::uInt chan=0; chan < cp.itsVisibility.ncolumn(); ++chan) {
                      temp += cp.itsVisibility(baseline,chan);
                 }
                 history(lastHistPos,beam,baseline) = temp / float(cp.itsVisibility.ncolumn());
                 
                 /*
                 // middle of the band
                 const casa::uInt chan = cp.itsVisibility.ncolumn() / 2;
                 ASKAPDEBUGASSERT(chan < cp.itsVisibility.ncolumn());
                 history(lastHistPos,beam,baseline) = cp.itsVisibility(baseline,chan);
                 */
            }
       }
       itsFiller->notifyWritingDone(buffer);
       // dump time history to a file
       std::ofstream os("visplot.dat");
       for (int i=0, curPos = wasWrapped ? lastHistPos + 1 : 0; i<nHistory; ++i,++curPos) {
            if (curPos >= nHistory) {
                curPos = 0;
                if (!wasWrapped) {
                    break;
                }
            }
            os<<bats[curPos]<<" ";
            for (int beam=0; beam < int(history.ncolumn()); ++beam) {
                for (casa::uInt baseline = 0; baseline < history.nplane(); ++baseline) {
                     os<<abs(history(curPos,beam,baseline))<<" "<<arg(history(curPos,beam,baseline))/casa::C::pi*180.<<" "
                       <<delayHistory(curPos,beam,baseline)*1e9<<" ";
                }
            }
            os<<std::endl;
            if (curPos == lastHistPos) {
                break;
            }
       }
    }
  } catch (const AskapError &ae) {
     ASKAPLOG_FATAL_STR(logger, "Writing thread (id="<<boost::this_thread::get_id()<<") is about to die: "<<ae.what());
     throw;
  }
}

} // namespace swcorrelator

} // namespace askap
