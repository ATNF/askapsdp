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
const int nSamples = 524288;

/// @brief constructor
/// @details
/// @param[in] nBeam number of beams
/// @param[in] nChan number of channels (cards)
BufferManager::BufferManager(const size_t nBeam, const size_t nChan) : itsNBuf(6*nBeam*nChan),
     itsBufferSize(2*nSamples + int(sizeof(BufferHeader)/sizeof(float))),
     itsBuffer(new float[(2*nSamples + int(sizeof(BufferHeader)/sizeof(float)))*itsNBuf]),
     itsStatus(itsNBuf, BUF_FREE),
     itsReadyBuffers(3, nChan, nBeam, -1)
{
   ASKAPCHECK(sizeof(BufferHeader) % sizeof(float) == 0, "Some padding is required");
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
  result.itsAnt3 = itsReadyBuffers(2, index.first, index.second);    
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
             for (casa::uInt ant = 0; ant < itsReadyBuffers.nrow(); ++ant) {
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
    if (ids.itsAnt3 >= 0) {
       releaseOneBuffer(ids.itsAnt3);
    }  
  }
  itsStatusCV.notify_all();
}
   
/// @brief notify that the buffer is ready for correlation
/// @details This method notifies the manager that the data buffer
/// has now been filled with information and is ready to be correlated.
/// This finishes operations with this buffer in the I/O thread.
/// @param[in] id buffer ID (should be non-negative)
void BufferManager::bufferFilled(const int id) const
{
  ASKAPDEBUGASSERT((id >= 0) && (id < itsNBuf));
  try {
    boost::lock_guard<boost::mutex> lock(itsStatusCVMutex);  
    ASKAPCHECK(itsStatus[id] == BUF_BEING_FILLED, "An attempt to release the buffer which is not being filled, status="<<
               itsStatus[id]);
    itsStatus[id] = BUF_READY;       
    const BufferHeader& hdr = header(id);
    if ((hdr.antenna >= itsReadyBuffers.nrow()) || (hdr.antenna < 0)) {
        ASKAPLOG_WARN_STR(logger, "Received data from unknown antenna "<<hdr.antenna<<" - ignoring");
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
    itsReadyBuffers(hdr.antenna, hdr.freqId, hdr.beam) = id;
  } catch (const BufferManager::HelperException &) {
    itsStatus[id] = BUF_FREE; 
    // no need to wake other threads up unnecessarily as long as we ignore this buffer
    return;    
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


} // namespace swcorrelator

} // namespace askap
