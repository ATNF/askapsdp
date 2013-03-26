/// @file 
///
/// @brief Thread which does correlation
/// @details This class holds shared pointers to the filler and the buffer
/// manager. The parallel thread extracts data corresponding to all three 
/// baselines, some spectral channel and beam, correlates them and passes
/// to the filler for writing. The filler and buffer manager manage 
/// synchronisation.
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

#include <swcorrelator/CorrWorker.h>
#include <swcorrelator/SimpleCorrelator.h>
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <boost/thread.hpp>

ASKAP_LOGGER(logger, ".corrworker");

namespace askap {

namespace swcorrelator {

/// @brief constructor
/// @details 
/// @param[in] filler shared pointer to a filler
/// @param[in] bm shared pointer to a buffer manager
CorrWorker::CorrWorker(const boost::shared_ptr<CorrFiller> &filler,
           const boost::shared_ptr<BufferManager> &bm) : itsFiller(filler), itsBufferManager(bm)
{
  ASKAPDEBUGASSERT(itsFiller);
  ASKAPDEBUGASSERT(itsBufferManager);
}             

/// @brief entry point for the parallel thread
void CorrWorker::operator()() 
{
  ASKAPLOG_INFO_STR(logger, "Correlator thread started, id="<<boost::this_thread::get_id());
  try {
    ASKAPDEBUGASSERT(itsFiller);
    ASKAPDEBUGASSERT(itsBufferManager);
    Simple3BaselineCorrelator<std::complex<float>, int> s3bc;
    // buffer size in complex floats
    const int size = (itsBufferManager->bufferSize() - int(sizeof(BufferHeader))) / sizeof(float) / 2;
    while (true) {
       // extract the first complete set of buffers
       BufferManager::BufferSet ids = itsBufferManager->getFilledBuffers();
       const uint64_t bat = itsBufferManager->header(ids.itsAnt1).bat;
       const int beam = itsBufferManager->header(ids.itsAnt1).beam;
       const int chan = itsBufferManager->header(ids.itsAnt1).freqId;
       // consistency checks
       ASKAPDEBUGASSERT(beam == int(itsBufferManager->header(ids.itsAnt2).beam));
       ASKAPDEBUGASSERT(beam == int(itsBufferManager->header(ids.itsAnt3).beam));
       ASKAPDEBUGASSERT(chan == int(itsBufferManager->header(ids.itsAnt2).freqId));
       ASKAPDEBUGASSERT(chan == int(itsBufferManager->header(ids.itsAnt3).freqId));
       ASKAPDEBUGASSERT(bat == itsBufferManager->header(ids.itsAnt2).bat);
       ASKAPDEBUGASSERT(bat == itsBufferManager->header(ids.itsAnt3).bat);
       const int frameOff_01 = int(itsBufferManager->header(ids.itsAnt1).frame) - int(itsBufferManager->header(ids.itsAnt2).frame);
       const int frameOff_12 = int(itsBufferManager->header(ids.itsAnt2).frame) - int(itsBufferManager->header(ids.itsAnt3).frame);
       const int frameOff_02 = int(itsBufferManager->header(ids.itsAnt1).frame) - int(itsBufferManager->header(ids.itsAnt3).frame);
       // for debugging
       if ((chan == 0) || (chan == 8)) {
          ASKAPLOG_INFO_STR(logger, "Frame difference (ant"<<ids.itsAnt1<<" - ant"<<ids.itsAnt2<<") is "<<
                            frameOff_01<<" for chan="<<chan);
          ASKAPLOG_INFO_STR(logger, "                 (ant"<<ids.itsAnt2<<" - ant"<<ids.itsAnt3<<") is "<<
                            frameOff_12<<" for chan="<<chan);
          ASKAPLOG_INFO_STR(logger, "                 (ant"<<ids.itsAnt1<<" - ant"<<ids.itsAnt3<<") is "<<
                            frameOff_02<<" for chan="<<chan);
       }
       // run correlation
       //s3bc.reset(0,0,0); // zero delays for now
       //s3bc.reset(0,0,+1); // for testing
       s3bc.reset(0,frameOff_01,frameOff_02); // derive offsets from frame differences
       //s3bc.reset(0,frameOff_01,frameOff_02+3); // derive offsets from frame differences
       //s3bc.reset(0,frameOff_01-1,frameOff_02-1); // derive offsets from frame differences
       //s3bc.reset(0,frameOff_01 + 1,frameOff_02 - (chan-8)); // derive offsets from frame differences
       s3bc.accumulate(itsBufferManager->data(ids.itsAnt1), itsBufferManager->data(ids.itsAnt2), 
                       itsBufferManager->data(ids.itsAnt3), size);
       itsBufferManager->releaseBuffers(ids);
       // store the result
       CorrProducts& cp = itsFiller->productsBuffer(beam, bat);
       cp.itsBAT = bat;
       const int baseline0 = cp.baseline(ids.itsAnt1, ids.itsAnt2);
       const int baseline1 = cp.baseline(ids.itsAnt2, ids.itsAnt3);
       const int baseline2 = cp.baseline(ids.itsAnt1, ids.itsAnt3);
       ASKAPDEBUGASSERT(baseline0 < int(cp.nBaseline()));
       ASKAPDEBUGASSERT(baseline1 < int(cp.nBaseline()));
       ASKAPDEBUGASSERT(baseline2 < int(cp.nBaseline()));
       
       if (chan==0) {
          ASKAPDEBUGASSERT(ids.itsAnt1 < int(cp.itsControl.nelements()));
          ASKAPDEBUGASSERT(ids.itsAnt2 < int(cp.itsControl.nelements()));
          ASKAPDEBUGASSERT(ids.itsAnt3 < int(cp.itsControl.nelements()));          
          cp.itsControl[ids.itsAnt1] = itsBufferManager->header(ids.itsAnt1).control;
          cp.itsControl[ids.itsAnt2] = itsBufferManager->header(ids.itsAnt2).control;
          cp.itsControl[ids.itsAnt3] = itsBufferManager->header(ids.itsAnt3).control;
       }
       // unflag this channel if frame offset is less than 100 by absolute value (it should be within a few steps); false is good here
          //cp.itsFlag.column(chan).set(false);
          cp.itsFlag(baseline0,chan) = (abs(frameOff_01) >= 100);
          cp.itsFlag(baseline1,chan) = (abs(frameOff_12) >= 100);
          cp.itsFlag(baseline2,chan) = (abs(frameOff_02) >= 100);
       //
       cp.itsVisibility(baseline0,chan) = s3bc.getVis12() / float(s3bc.nSamples12()!=0 ? s3bc.nSamples12() : 1.);
       cp.itsVisibility(baseline1,chan) = s3bc.getVis23() / float(s3bc.nSamples23()!=0 ? s3bc.nSamples23() : 1.);
       cp.itsVisibility(baseline2,chan) = s3bc.getVis13() / float(s3bc.nSamples13()!=0 ? s3bc.nSamples13() : 1.);       
       itsFiller->notifyProductsReady(beam);
    }
  } catch (const boost::thread_interrupted &) { 
     ASKAPLOG_INFO_STR(logger, "Correlator thread (id="<<boost::this_thread::get_id()<<") has been interrupted and is about to finish");
  } catch (const AskapError &ae) {
     ASKAPLOG_FATAL_STR(logger, "Correlator thread (id="<<boost::this_thread::get_id()<<") is about to die: "<<ae.what());
     throw;
  }  
}


} // namespace swcorrelator

} // namespace askap

