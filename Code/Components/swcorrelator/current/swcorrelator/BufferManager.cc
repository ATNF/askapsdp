/// @file 
///
/// @brief manages buffers for broad-band data
/// @details This class manages buffers for broad-band data 
/// can keeps track of the current status (i.e. free, filled, 
/// being reduced) providing the required syncronisation between
/// parallel threads accessing the buffers. The number of buffers should be
/// at least twice the number of beams * antennas * cards.
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

#include <swcorrelator/BufferManager.h>
#include <askap/AskapError.h>
#include <boost/thread/thread.hpp>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>


ASKAP_LOGGER(logger, ".swcorrelator");

namespace askap {

namespace swcorrelator {

// number of samples per buffer
//const int nSamples = 4194304;
//const int nSamples = 524288;
const int nSamples = 1048576;


/// @brief get the number of samples
/// @details This number is hard coded (defined by the data communication
/// protocol). It is handy to have it defined in a single place and 
/// access via this method.
/// @return number of samples (each is complex float)
int BufferManager::NumberOfSamples() {
  return nSamples;
}

/// @brief constructor
/// @details
/// @param[in] nBeam number of beams
/// @param[in] nChan number of channels (cards)
/// @param[in[ hdrProc optionan shared pointer to the header preprocessor
BufferManager::BufferManager(const size_t nBeam, const size_t nChan, 
     const boost::shared_ptr<HeaderPreprocessor> &hdrProc) : itsNBuf(6*nBeam*nChan),
     itsBufferSize(2*nSamples + int(sizeof(BufferHeader)/sizeof(float))),
     itsBuffer(new float[(2*nSamples + int(sizeof(BufferHeader)/sizeof(float)))*itsNBuf]),
     itsStatus(itsNBuf, BUF_FREE),
     itsReadyBuffers(3, nChan, nBeam, -1), itsHeaderPreprocessor(hdrProc),
     itsDuplicate2nd(false)
{
   ASKAPCHECK(sizeof(BufferHeader) % sizeof(float) == 0, "Some padding is required");
   ASKAPCHECK(sizeof(std::complex<float>) == 2*sizeof(float), "std::complex<float> is not just two floats!");
}
   
/// @brief obtain a header for the given buffer
/// @details
/// @param[in] id buffer ID (should be non-negative)
/// @return const reference to the header of the given buffer
const BufferHeader& BufferManager::header(const int id) const
{
   ASKAPDEBUGASSERT((id >= 0) && (id<itsNBuf));
   BufferHeader *bh = (BufferHeader*)(itsBuffer.get() + id * itsBufferSize);
   return *bh;
}
   
/// @brief access to the data part of the buffer
/// @details
/// @param[in] id buffer ID (should be non-negative)
/// @return pointer to the first data element of the buffer
std::complex<float>* BufferManager::data(const int id) const 
{
   ASKAPDEBUGASSERT((id >= 0) && (id<itsNBuf));
   float *start = itsBuffer.get() + id * itsBufferSize + int(sizeof(BufferHeader)/sizeof(float));
   return (std::complex<float>*)(start);
}

/// @brief access to the buffer as a whole
/// @details This method is intended to be used with the actual
/// receiving code (which doesn't discriminate between the header
/// and the data)
/// @param[in] id buffer ID (should be non-negative)
/// @return pointer to the buffer
void* BufferManager::buffer(const int id) const
{
   ASKAPDEBUGASSERT((id >= 0) && (id<itsNBuf));
   float *start = itsBuffer.get() + id * itsBufferSize;
   return (void*)start;   
}
   
/// @brief size of a single buffer
/// @return size of a single buffer in bytes
int BufferManager::bufferSize() const
{
  return itsBufferSize * sizeof(float);
}
   
/// @brief obtain a buffer to receive data
/// @details This method return an ID of a free buffer used to
/// receive the data. If no free buffer is available (i.e. an
/// overflow situation), a negative value is returned.
/// @return an ID of the buffer
int BufferManager::getBufferToFill() const
{
  boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);  
  for (int id = 0; id < itsNBuf; ++id) {
     if (itsStatus[id] == BUF_FREE) {
         itsStatus[id] = BUF_BEING_FILLED;
         return id;
     }
  }
  return -1;
}
   
/// @brief get filled buffers for a matching channel + beam
/// @details This method returns the first available set of
/// completely filled buffers corresponding to the same channel
/// and beam. The calling thread is blocked until a suitable set
/// is available for correlation.
BufferManager::BufferSet BufferManager::getFilledBuffers() const
{
  boost::unique_lock<boost::mutex> lock(itsStatusCVMutex);
  std::pair<int,int> index;
  while (findCompleteSet(index)) {  
     itsStatusCV.wait(lock);
  }
  ASKAPDEBUGASSERT(itsReadyBuffers.nrow() == 3);
  BufferManager::BufferSet result;
  result.itsAnt1 = itsReadyBuffers(0, index.first, index.second);
  result.itsAnt2 = itsReadyBuffers(1, index.first, index.second);
  result.itsAnt3 = itsReadyBuffers(itsDuplicate2nd ? 1 : 2, index.first, index.second);    
  for (casa::uInt ant = 0; ant < itsReadyBuffers.nrow(); ++ant) {
       const int id = itsReadyBuffers(ant, index.first, index.second);
       ASKAPDEBUGASSERT(id < itsNBuf);
       itsStatus[id] = BUF_BEING_PROCESSED;      
       itsReadyBuffers(ant, index.first, index.second) = -1;
  }
  return result;
}

/// @brief find a complete set of data 
/// @details We process all antennas simultaneously (for speed). This method
/// finds channel/beam numbers which are ready to be correlated
/// @param[out] index channel/beam pair (output)
/// @return true if nothing has been found so far, false otherwise
/// @note The method assumes that a lock has been acquired
bool BufferManager::findCompleteSet(std::pair<int,int> &index) const
{
   for (casa::uInt chan = 0; chan < itsReadyBuffers.ncolumn(); ++chan) {
        for (casa::uInt beam = 0; beam < itsReadyBuffers.nplane(); ++beam) {
             bool isGood = true;
             for (casa::uInt ant = 0; (itsDuplicate2nd ? ant + 1 : ant) < itsReadyBuffers.nrow(); ++ant) {
                  if (itsReadyBuffers(ant,chan,beam) < 0) {
                      isGood = false;
                      break;
                  }
             }
             if (isGood) {
                 index.first = chan;
                 index.second = beam;
                 return false;
             }
        }
   }  
   index.first = -1;
   index.second = -1;
   return true;
}

/// @brief get one filled buffer
/// @details This method is only used with the capture, correlation
/// always accesses 3 buffers at once
/// @return a buffer ready to be dumped into disk
int BufferManager::getFilledBuffer() const
{
  boost::unique_lock<boost::mutex> lock(itsStatusCVMutex);
  while (true) {
     for (int id = 0; id < itsNBuf; ++id) {
         if (itsStatus[id] == BUF_READY) {
             return id;
         }
     }
     itsStatusCV.wait(lock);
  }  
}
   
/// @brief release one buffer
/// @details This method notifies the manager that data dump is 
/// now complete and the data buffers can now be released.
/// @param[in] id the buffer to release
/// @note the correlation uses an overloaded version of this 
/// method which releases 3 buffers in a row
void BufferManager::releaseBuffers(const int id) const
{
  {
    boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);  
    if (id >= 0) {
        releaseOneBuffer(id);
    }
  }
  itsStatusCV.notify_all();
}

   
/// @brief release the buffers
/// @details This method notifies the manager that correlation is
/// now complete and the data buffers can now be released.
/// @param[in] ids buffer set to release
void BufferManager::releaseBuffers(const BufferSet &ids) const
{
  {
    boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);  
    if (ids.itsAnt1 >= 0) {
        releaseOneBuffer(ids.itsAnt1);
    }
    if (ids.itsAnt2 >= 0) {
        releaseOneBuffer(ids.itsAnt2);
    }
    if (ids.itsAnt3 >= 0 && !itsDuplicate2nd) {
       releaseOneBuffer(ids.itsAnt3);
    }  
  }
  itsStatusCV.notify_all();
}

/// @brief optional index substitution
/// @details We want to be quite flexible and allow various substitutions of
/// indices (e.g. call beam an antenna or renumber them). This method modifies
/// the header in place for this purpose
/// @param[in] id buffer ID (should be non-negative)
/// @note it is assumed that this method called from bufferFilled and the appropriate
/// mutex lock has been obtained.
/// @return true if the current buffer has to be rejected (no mapping available)
bool BufferManager::preprocessIndices(const int id) const
{
   if (itsHeaderPreprocessor) {
       // can't use the header method here because it's const, but it is easy enough to replicate
       BufferHeader &hdr = *((BufferHeader*)buffer(id));
       return itsHeaderPreprocessor->updateHeader(hdr);
   } 
   // preprocessor is not set up
   return false;
}
   
/// @brief notify that the buffer is ready for correlation
/// @details This method notifies the manager that the data buffer
/// has now been filled with information and is ready to be correlated.
/// This finishes operations with this buffer in the I/O thread.
/// @param[in] id buffer ID (should be non-negative)
void BufferManager::bufferFilled(const int id) const
{
  ASKAPDEBUGASSERT((id >= 0) && (id < itsNBuf));
  { // mutex is locked in this block only
    boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);  
    try {
      ASKAPCHECK(itsStatus[id] == BUF_BEING_FILLED, "An attempt to release the buffer which is not being filled, status="<<
                 itsStatus[id]);
      itsStatus[id] = BUF_READY;       
      //((BufferHeader*)buffer(id))->beam-=2;
      if (preprocessIndices(id)) {
          // we could've just defined hdr above if-operator, but it is neater this way because the content of
          // hdr may change after preprocessIndices.
          const BufferHeader& hdr = header(id);
          ASKAPLOG_WARN_STR(logger, "Received data which are not mapped to any valid antenna/beam/frequency ("<<
              hdr.antenna<<","<<hdr.beam<<","<<hdr.freqId<<") - ignoring");
          throw BufferManager::HelperException();
      }
      const BufferHeader& hdr = header(id);
      if ((hdr.antenna >= itsReadyBuffers.nrow()) || (hdr.antenna < 0)) {
          ASKAPLOG_WARN_STR(logger, "Received data from unknown antenna "<<hdr.antenna<<" - ignoring");
          throw BufferManager::HelperException();
      }
      if ((hdr.antenna + 1 == itsReadyBuffers.nrow()) && itsDuplicate2nd) {
          ASKAPLOG_WARN_STR(logger, "The correlator is configured to duplicate data from 2nd antenna as if they would come from the 3rd, ignoring antenna "<<hdr.antenna);
          throw BufferManager::HelperException();
      }
      if ((hdr.freqId >= itsReadyBuffers.ncolumn()) || (hdr.freqId < 0)) {
          ASKAPLOG_WARN_STR(logger, "Received data from unknown channel (card) "<<hdr.freqId<<" - ignoring");
          throw BufferManager::HelperException();
      }
      if ((hdr.beam >= itsReadyBuffers.nplane()) || (hdr.beam < 0)) {
          ASKAPLOG_WARN_STR(logger, "Received data from unknown beam "<<hdr.beam<<" - ignoring");
          throw BufferManager::HelperException();
      }
      // check that buffers which have already been filled correspond to the same bat
      // release those buffers which are not
      const uint64_t newBAT = hdr.bat;
      for (int ant = 0; ant<int(itsReadyBuffers.nrow()); ++ant) {
           const int thisID = itsReadyBuffers(ant, hdr.freqId, hdr.beam);
           if (thisID >= 0) {
               ASKAPDEBUGASSERT(thisID < itsNBuf);
               if (newBAT < header(thisID).bat) {
                   ASKAPLOG_WARN_STR(logger, "Not keeping up - received data for antenna "<<hdr.antenna<<" which are too old, ignoring");
                   throw BufferManager::HelperException();
               }
               if (newBAT > header(thisID).bat) {
                   if (itsStatus[thisID] == BUF_READY) {
                       ASKAPLOG_WARN_STR(logger, "Incomplete old data detected in buffer "<<thisID<<" corresponding to antenna "<<
                              ant<<", beam "<<hdr.beam<<", channel "<<hdr.freqId<<" - cleaning up");
                       itsReadyBuffers(ant, hdr.freqId, hdr.beam) = -1;
                       itsStatus[thisID] = BUF_FREE;
                   } else {
                      ASKAPDEBUGASSERT(itsStatus[thisID] == BUF_BEING_PROCESSED);
                      ASKAPLOG_WARN_STR(logger, "Not keeping up - the data in buffer "<<thisID<<" corresponding to antenna "<<
                              ant<<", beam "<<hdr.beam<<", channel "<<hdr.freqId<<" are still being processed, ingore new data in buffer "<<id);
                      throw BufferManager::HelperException();
                   }
               }
           }
      }
      itsReadyBuffers(hdr.antenna, hdr.freqId, hdr.beam) = id;      
      // for debugging
      if (hdr.freqId == 0) {
          ASKAPLOG_INFO_STR(logger, "Header for ant/chan/beam="<<hdr.antenna<<"/"<<hdr.freqId<<"/"<<hdr.beam<<" corresponds to frame="<<hdr.frame<<" and bat="<<hdr.bat);
      }
      //
    } catch (const BufferManager::HelperException &) {
      itsStatus[id] = BUF_FREE;          
    }
  }
  itsStatusCV.notify_all();
}

/// @brief release single buffer after correlation
/// @details This method is called from releaseBuffers for each individual
/// buffer id. It is assumed that the exclusive lock on mutex has already 
/// been acquired.
/// @param[in] id buffer ID to release
void BufferManager::releaseOneBuffer(const int id) const
{
   ASKAPDEBUGASSERT(id < itsNBuf);
   itsStatus[id] = BUF_FREE;   
}


/// @brief control itsDuplicate2nd flag
/// @details If this flag is true, the data from the second antenna (id=1)
/// will be used as the data from the third antenna (id=2) allowing operations
/// in the single baseline case.
/// @param[in] duplicate the new value of the flag
/// @note The optional substitution is done before duplication of the antenna
void BufferManager::duplicate2nd(const bool duplicate)
{
   itsDuplicate2nd = duplicate;
}


} // namespace swcorrelator

} // namespace askap
