/// @file
///
/// Actual algorithm implementing delay solver tool 
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
///

#include <delaysolver/DelaySolverImpl.h>
#include <askap/AskapLogging.h>
#include <askap/AskapUtil.h>
ASKAP_LOGGER(logger, ".delaysolver.DelaySolverImpl");

#include <askap/AskapError.h>
#include <casa/Arrays/MatrixMath.h>

// std
#include <set>

using namespace askap;
using namespace askap::utils;

/// @brief constructor
/// @param[in] targetRes target spectral resolution in Hz, data are averaged to match the desired resolution 
/// note, integral number of channels are averaged.
/// @param[in] pol polarisation index to use
/// @param[in] ampCutoff if positive, amplitudes above ampCutoff will be flagged
/// @param[in] refAnt reference antenna index   
DelaySolverImpl::DelaySolverImpl(double targetRes, casa::Stokes::StokesTypes pol, float ampCutoff, casa::uInt refAnt) :
   itsTargetRes(targetRes), itsPol(pol), itsAmpCutoff(ampCutoff), itsRefAnt(refAnt), itsNAvg(0u), itsDelayEstimator(targetRes),
   itsChanToAverage(1u) 
{
  ASKAPCHECK(itsTargetRes > 0, "Target spectral resolution should be positive, you have "<<itsTargetRes<<" Hz");
} 
    
/// @brief set baselines to exclude
/// @details An empty vector configures the class to take all available baselines into account.
/// @param[in] bsln vector with pair of indices representing baselines to exclude from the solution
void DelaySolverImpl::excludeBaselines(const casa::Vector<std::pair<casa::uInt, casa::uInt> > &bsln) 
{
  itsExcludedBaselines.assign(bsln.copy());
}

/// @brief helper method to check that all channels/rows are flagged
/// @param[in] flags matrix with flags
/// @return true, if all channels and rows are flagged
bool DelaySolverImpl::checkAllFlagged(const casa::Matrix<bool> &flags)
{
   for (casa::uInt row=0; row<flags.nrow(); ++row) {
        for (casa::uInt chan=0; chan<flags.ncolumn(); ++chan) {
             if (!flags(row,chan)) {
                 return false;
             }
        }
   }
   return true;
}

     
/// @brief process one data accessor
/// @param[in] acc data accessor to process
void DelaySolverImpl::process(const accessors::IConstDataAccessor &acc)
{
   const casa::Vector<casa::Stokes::StokesTypes>& stokes = acc.stokes();
   casa::uInt pol2use = stokes.nelements();
   for (pol2use = 0; pol2use < stokes.nelements(); ++pol2use) {
        if (stokes[pol2use] == itsPol) {
            break;
        }
   }
   ASKAPCHECK(pol2use < stokes.nelements(), "Unable to find "<<casa::Stokes::name(itsPol)<<" polarisation product in the data");
   
   const casa::Matrix<bool>& flags = acc.flag().xyPlane(pol2use);   
   if (checkAllFlagged(flags)) {
       return;
   }
   if (itsFreqAxis.nelements() == 0) {
       // this is the first time stamp, resize buffers, etc
       itsFreqAxis = acc.frequency();
       itsAnt1IDs = acc.antenna1();
       itsAnt2IDs = acc.antenna2();
       ASKAPDEBUGASSERT(itsAnt1IDs.nelements() == itsAnt2IDs.nelements());
       // determine the number of channels to average based on the target and actual resolution
       ASKAPCHECK(itsFreqAxis.nelements() > 1, "Need at least two spectral channels, you have "<<acc.nChannel());
       const double actualRes = (itsFreqAxis[itsFreqAxis.nelements() - 1] - itsFreqAxis[0]) / double(itsFreqAxis.nelements() - 1);
       ASKAPCHECK(abs(actualRes) > 0, "Unable to determine spectral resolution of the data");
       if (itsTargetRes > abs(actualRes)) {
           ASKAPDEBUGASSERT(itsTargetRes > 0);
           itsChanToAverage = casa::uInt(itsTargetRes / abs(actualRes));
       } 
       ASKAPLOG_INFO_STR(logger, "Averaging "<<itsChanToAverage<<" consecutive spectral channels");
       ASKAPDEBUGASSERT(itsChanToAverage > 0);
       itsDelayEstimator.setResolution(actualRes * itsChanToAverage);
       const casa::uInt targetNChan = acc.nChannel() / itsChanToAverage;
       ASKAPCHECK(targetNChan > 1, "Too few spectral channels remain after averaging: in="<<acc.nChannel()<<" out="<<targetNChan);
       itsSpcBuffer.resize(acc.nRow(), targetNChan);
       itsAvgCounts.resize(acc.nRow(), targetNChan);
       itsSpcBuffer.set(casa::Complex(0.,0.));
       itsAvgCounts.set(0u);
   } else {
       if (itsFreqAxis.nelements() != acc.nChannel()) {
           ASKAPLOG_WARN_STR(logger, "The number of frequency channels has been changed, was "<<itsFreqAxis.nelements()<<" now "
                             << acc.nChannel()<<", ignoring");
           return;
       }
       if (itsAnt1IDs.nelements() != acc.nRow()) {
           ASKAPLOG_WARN_STR(logger, "The number of rows has been changed, was "<<itsAnt1IDs.nelements()<<" now "
                             << acc.nRow()<<", ignoring");
           return;
       }
       for (casa::uInt row = 0; row < acc.nRow(); ++row) {
            if (itsAnt1IDs[row] != acc.antenna1()[row]) {
                ASKAPLOG_WARN_STR(logger, "Antenna 1 index has been changed for row ="<<row<<", was "<<itsAnt1IDs[row]<<
                                  " now "<<acc.antenna1()[row]<<", ignoring");
                return;                  
            }                     
            if (itsAnt2IDs[row] != acc.antenna2()[row]) {
                ASKAPLOG_WARN_STR(logger, "Antenna 2 index has been changed for row ="<<row<<", was "<<itsAnt2IDs[row]<<
                                  " now "<<acc.antenna2()[row]<<", ignoring");
                return;                  
            }                     
       }
   }
   
   const casa::Matrix<casa::Complex> vis = acc.visibility().xyPlane(pol2use);   
   for (casa::uInt row = 0; row < acc.nRow(); ++row) {
        const casa::Vector<casa::Complex> thisRowVis = vis.row(row);
        const casa::Vector<bool> thisRowFlags = flags.row(row);
        ASKAPDEBUGASSERT(itsSpcBuffer.nrow() == acc.nRow());
        ASKAPDEBUGASSERT(itsAvgCounts.nrow() == acc.nRow());        
        casa::Vector<casa::Complex> thisBufRowVis = itsSpcBuffer.row(row);
        casa::Vector<casa::uInt> thisRowCounts = itsAvgCounts.row(row);
        
        ASKAPDEBUGASSERT(thisRowVis.nelements() == thisRowFlags.nelements());
        for (casa::uInt chan = 0, index = 0; chan < thisBufRowVis.nelements(); ++chan) {
             for (casa::uInt ch2 = 0; ch2 < itsChanToAverage; ++ch2,++index) {
                  ASKAPDEBUGASSERT(index < thisRowFlags.nelements());
                  if (!thisRowFlags[index]) {
                      const casa::Complex curVis = thisRowVis[index];
                      if ((itsAmpCutoff < 0) || (abs(curVis) < itsAmpCutoff)) {
                          thisBufRowVis[chan] += curVis;
                          ++thisRowCounts[chan];
                      }
                  }
             }   
        }
   }
   ++itsNAvg;
}
    
/// @brief solve for antenna-based delays
/// @details This method estimates delays for all baselines and then solves for
/// antenna-based delays honouring baselines to be excluded. 
/// @return a vector with one delay per antenna (antennas are in the index-increasing order).
casa::Vector<double> DelaySolverImpl::solve() const
{
  ASKAPCHECK(itsNAvg > 0, "No valid data found. At least one chunk of data have to be processed before delays can be estimated");
  ASKAPCHECK(itsFreqAxis.nelements() > 1, "Unable to estimate delays from monochromatic data");
  
  const casa::uInt nAnt = casa::max(casa::max(itsAnt1IDs), casa::max(itsAnt2IDs)) + 1;
  
  ASKAPLOG_INFO_STR(logger, "Using "<<itsNAvg<<" cycles to estimate delays for "<<nAnt<<" antennas; reference = "<<itsRefAnt);
  // build a set of baselines (rows) to exclude
  std::set<casa::uInt> rows2exclude;
  ASKAPDEBUGASSERT(itsAnt1IDs.nelements() == itsAnt2IDs.nelements());
  for (casa::uInt row=0; row<itsAnt1IDs.nelements(); ++row) {
       for (casa::uInt bsln = 0; bsln < itsExcludedBaselines.nelements(); ++bsln) {
            if ((itsExcludedBaselines[bsln].first == itsAnt1IDs[row]) && 
                (itsExcludedBaselines[bsln].second == itsAnt2IDs[row])) {
                 rows2exclude.insert(row);
            }
       }
  }
  ASKAPLOG_INFO_STR(logger, "Using "<<itsAnt1IDs.nelements() - rows2exclude.size()<<" rows(baselines) out of "<<
                             itsAnt1IDs.nelements()<<" available in the dataset");
  
  ASKAPDEBUGASSERT(itsAnt1IDs.nelements() == itsSpcBuffer.nrow());
  casa::Vector<double> delays(itsSpcBuffer.nrow()+1,0.);
  casa::Matrix<double> dm(delays.nelements(),nAnt);
  for (casa::uInt bsln = 0; bsln < itsSpcBuffer.nrow(); ++bsln) {
       if (rows2exclude.find(bsln) == rows2exclude.end()) {
           casa::Vector<casa::Complex> buf = itsSpcBuffer.row(bsln).copy();
           const casa::Vector<casa::uInt> thisRowCounts = itsAvgCounts.row(bsln);
           ASKAPDEBUGASSERT(buf.nelements() == thisRowCounts.nelements());           
           for (casa::uInt chan=0; chan < buf.nelements(); ++chan) {
                if (thisRowCounts[chan] > 0) {
                    buf[chan] /= float(thisRowCounts[chan]);
                } else {
                    if (chan > 0) {
                        // don't really have much better way to guess the phase of the flagged channel
                        // We could've interpolate, but unwrapping phase may be difficult, especially if
                        // more than one channel is flagged
                        buf[chan] = buf[chan - 1];
                    }
                }
           }
           delays[bsln] = itsDelayEstimator.getDelay(buf);
           // now fill the design matrix
           const casa::uInt ant1 = itsAnt1IDs[bsln];
           ASKAPDEBUGASSERT(ant1 < dm.ncolumn()); 
           const casa::uInt ant2 = itsAnt2IDs[bsln];
           ASKAPDEBUGASSERT(ant2 < dm.ncolumn());
           ASKAPDEBUGASSERT(bsln < dm.nrow()); 
           if (ant1 != itsRefAnt) {               
               dm(bsln,ant1) = 1.;
           }
           if (ant2 != itsRefAnt) {
               dm(bsln,ant2) = -1.;
           }              
       }
  }
  // condition for the reference antenna (ref. delay is set in the last element of delays)
  dm(itsSpcBuffer.nrow(),itsRefAnt) = 1.;
  
  // just do an explicit LSQ fit. We could've used SVD invert here.
  casa::Matrix<double> nm = transpose(dm) * dm;
  
  casa::Vector<double> result = invert(nm) * (dm * delays);
  
  return result;  
}
