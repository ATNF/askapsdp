/// @file 
///
/// @brief MS filler and the result buffer manager
/// @details This class manages a double buffer for the resulting visibilities and
/// flags. When the BAT timestamp changes, it stores the previous buffer.
/// It is intended to be run in a parallel thread.
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

#include <swcorrelator/CorrFiller.h>
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>

ASKAP_LOGGER(logger, ".corrfiller");

namespace askap {

namespace swcorrelator {


/// @brief constructor, sets up the filler
/// @details Configuration is done via the parset
/// @param[in] parset parset file with configuration info
CorrFiller::CorrFiller(const LOFAR::ParameterSet &parset) :
   itsNAnt(parset.getInt32("nant",3)), itsNBeam(parset.getInt32("nbeam",1)),
   itsNChan(parset.getInt32("nchan",1)), itsFlushStatus(false,false), itsFirstActive(true), 
   itsActiveBAT(uint64_t(-1)), itsReadyToWrite(false), itsSwapHandled(false)
{
  ASKAPCHECK(itsNAnt == 3, "Only 3 antennas are supported at the moment");
  ASKAPCHECK(itsNChan > 0, "Number of channels should be positive");
  ASKAPCHECK(itsNBeam > 0, "Number of beams should be positive");
  
  ASKAPLOG_INFO_STR(logger, "Initialise filler for "<<nAnt()<<" antennas and up to "<<nBeam()<<" beams and "<<nChan()<<" channels(cards)");
  
  itsCorrProducts.resize(2*nBeam());
  for (int buf=0; buf<int(itsCorrProducts.size()); ++buf) {
       itsCorrProducts[buf].reset(new CorrProducts(nChan(), buf % nBeam()));
  }
  itsFillStatus.resize(nBeam(),false);  
  // initialisation of MS will come here
}

/// @brief shutdown the filler
/// @detais This method is effectively a destructor, which can be
/// called more explicitly. It stops the writing thread and
/// closes the MS which is currently being written.
void CorrFiller::shutdown()
{
  // MS should be flushed to disk and closed here
}

/// @brief get buffer to write it to MS
/// @details This method is intended to be called from the writing thread. It obtains
/// a buffer corresponding to the given beam. It is assumed the required locks have
/// already been obtained.
/// @param[in] beam beam of interest
/// @oaram[in] useFirst true, if the first set of buffers is flushed, false otherwise
CorrProducts& CorrFiller::getProductsToWrite(const int beam, const bool useFirst) const
{
  const int buf = beam + (useFirst ? 0 : nBeam());
  ASKAPDEBUGASSERT((buf >= 0) && (buf < int(itsCorrProducts.size())));
  boost::shared_ptr<CorrProducts> cp = itsCorrProducts[buf];
  ASKAPDEBUGASSERT(cp);
  return *cp;
}

/// @brief get job for writing 
/// @details This method is intended to be called from the writing thread. It acquires
/// the required locks (and block the thread until it happens). The method returns
/// the flag showing which buffer set is to be written.
/// @return true, if the first buffer is to be written, false otherwise
bool CorrFiller::getWritingJob() 
{
  boost::unique_lock<boost::mutex> lock(itsStatusCVMutex);
  while (!itsReadyToWrite) {
     itsStatusCV.wait(lock);
  }
  itsReadyToWrite = false;
  const bool useFirst = !itsFirstActive;
  if (useFirst) {
      ASKAPDEBUGASSERT(!itsFlushStatus.first);
      itsFlushStatus.first = true;
  } else {
      ASKAPDEBUGASSERT(!itsFlushStatus.second);
      itsFlushStatus.second = true;
  }
  return useFirst;
}
  
/// @brief notify that the writing job has been finished
/// @details This method is intended to be called from the writing thread. It releases
/// the whole set of buffers unlocking them for the corelation threads.
/// @param[in] isFirst true, if the first set of buffers is released, false otherwise
void CorrFiller::notifyWritingDone(const bool useFirst)
{
  boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);
  if (useFirst) {
      ASKAPDEBUGASSERT(itsFlushStatus.first);
      itsFlushStatus.first = false;
  } else {
      ASKAPDEBUGASSERT(itsFlushStatus.second);
      itsFlushStatus.second = false;
  }  
}

/// @brief notify that the new data are about to be received
/// @details This method is intended to be called from correlator threads when
/// a new piece of data is about to be stored in a buffer. It triggers write and
/// a buffer swap if the new BAT is different from that of the buffer.
/// @param[in] bat time stamp of the new data
void CorrFiller::notifyOfNewData(const uint64_t bat)
{
  bool needToWait = false;
  {
    boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);
    if (itsActiveBAT == uint64_t(-1)) {
        // this is the first use, assign the first buffer
        itsActiveBAT = bat;
        itsFirstActive = true;
        return;
    }
    if (bat == itsActiveBAT) {
        return;
    }  
    if (bat < itsActiveBAT) {
        ASKAPLOG_FATAL_STR(logger, "New BAT="<<bat<<" is before the last processed BAT="<<itsActiveBAT);
        return;
    }
    if (itsSwapHandled) {
        needToWait = true;
    } else {
       // we have to trigger buffer swap and writing       
       itsSwapHandled = true;
    }
  } // this unlocks the mutex allowing for fill operation to finish (if we're not keeping up)
  waitFillCompletion();
  // finalising the swap if necessary
  if (!needToWait) {
      // this thread handles swap
      boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);
      if (itsFlushStatus.first || itsFlushStatus.second) {
          ASKAPLOG_FATAL_STR(logger, "Not keeping up (current bat="<<itsActiveBAT<<", new bat="<<bat
               <<") data will be corrupted in some way");
      } else {
         itsReadyToWrite = true;
         itsFirstActive = !itsFirstActive;
         itsActiveBAT = bat;
         // prepare the new buffers (i.e. all data are flagged by default
         const int offset = itsFirstActive ? 0 : nBeam();
         for (int beam = 0; beam < nBeam(); ++beam) {
              boost::shared_ptr<CorrProducts> cp = itsCorrProducts[beam + offset];
              ASKAPDEBUGASSERT(cp);
              cp->init(bat);
         }
      }
      itsSwapHandled = false;
  }   
  // one more if-statement because we don't need mutex lock here
  if (needToWait) {
     // this thread is not handling the swap, waiting for the flag to be released
     boost::unique_lock<boost::mutex> lock(itsStatusCVMutex);
     while (itsSwapHandled) {
        itsStatusCV.wait(lock);
     }
  } else {
     // this thread has handled the swap, notify via condition variable
     itsStatusCV.notify_all();
  }
}

/// @brief wait for all fill operations to complete
/// @details This method waits for all buffer fill operations to complete,
/// this is necessary before the buffer swap could be initiated and the
/// current buffer could be transfered to the writing thread to store.
void CorrFiller::waitFillCompletion()
{
  boost::unique_lock<boost::mutex> lock(itsStatusCVMutex);
  bool keepGoing = true;
  while (keepGoing) {
    keepGoing = false;
    for (int beam = 0; beam < int(itsFillStatus.size()); ++beam) {
       if (itsFillStatus[beam]) {
           keepGoing = true;
           break;
       }
    }
    itsStatusCV.wait(lock);
  }
}

/// @brief get buffer to be fileld with new data
/// @details This method is intended to be called from correlator threads when
/// new visibility data are ready to be stored
/// @param[in] beam beam index
/// @param[in] bat time stamp
/// @return reference to the buffer
/// @note it calls notifyOfNewData
CorrProducts& CorrFiller::productsBuffer(const int beam, const uint64_t bat)
{
  ASKAPDEBUGASSERT((beam>=0) || (beam < nBeam()));
  notifyOfNewData(bat);
  boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);
  if (bat != itsActiveBAT) {
      ASKAPLOG_FATAL_STR(logger, "Not keeping up buffer swap has been initiated while the result was copied");
  } else {
      if (itsFillStatus[beam]) {
          ASKAPLOG_FATAL_STR(logger, "The buffer for beam="<<beam<<" and bat="<<bat<<" is already being filled");
      } 
      itsFillStatus[beam] = true;
  }  
  const int offset = itsFirstActive ? 0 : nBeam();
  boost::shared_ptr<CorrProducts> cp = itsCorrProducts[beam + offset];
  ASKAPDEBUGASSERT(cp);
  return *cp;
}
  
/// @brief notify that the buffer has been filled with data
/// @param[in] beam beam index
void CorrFiller::notifyProductsReady(const int beam)
{
  ASKAPDEBUGASSERT((beam>=0) || (beam < nBeam()));
  {
    boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);
    if (!itsFillStatus[beam]) {
        ASKAPLOG_FATAL_STR(logger, "The buffer for beam="<<beam<<" does not appear to be locked for filling");
    } 
    itsFillStatus[beam] = false;    
  }
  itsStatusCV.notify_all();
}


} // namespace swcorrelator

} // namespace askap

