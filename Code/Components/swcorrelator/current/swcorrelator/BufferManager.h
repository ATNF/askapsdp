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

#ifndef ASKAP_SWCORRELATOR_BUFFER_MANAGER
#define ASKAP_SWCORRELATOR_BUFFER_MANAGER

#include <swcorrelator/BufferHeader.h>

#include <boost/thread/thread.hpp>
#include <boost/scoped_array.hpp>


#include <complex>
#include <vector>
#include <utility>
#include <stdexcept>

#include <casa/Arrays/Cube.h>

namespace askap {

namespace swcorrelator {

/// @brief manages buffers for broad-band data
/// @details This class manages buffers for broad-band data 
/// can keeps track of the current status (i.e. free, filled, 
/// being reduced) providing the required syncronisation between
/// parallel threads accessing the buffers. The number of buffers should be
/// at least twice the number of beams * antennas * cards.
/// @ingroup swcorrelator
class BufferManager {
public:
   /// @brief 3 buffers corresponding to the same channel and beam
   struct BufferSet {
      BufferSet() : itsAnt1(-1), itsAnt2(-1), itsAnt3(-1) {}
      int itsAnt1;
      int itsAnt2;
      int itsAnt3;
   };
   
   enum BufferStatus {
     BUF_FREE = 0,
     BUF_BEING_FILLED,
     BUF_READY,
     BUF_BEING_PROCESSED
   };
   
   /// @brief constructor
   /// @details
   /// @param[in] nBeam number of beams
   /// @param[in] nChan number of channels (cards)
   BufferManager(const size_t nBeam, const size_t nChan);
   
   /// @brief obtain a header for the given buffer
   /// @details
   /// @param[in] id buffer ID (should be non-negative)
   /// @return const reference to the header of the given buffer
   const BufferHeader& header(const int id) const;
   
   /// @brief access to the data part of the buffer
   /// @details
   /// @param[in] id buffer ID (should be non-negative)
   /// @return pointer to the first data element of the buffer
   std::complex<float>* data(const int id) const;  
   
   /// @brief access to the buffer as a whole
   /// @details This method is intended to be used with the actual
   /// receiving code (which doesn't discriminate between the header
   /// and the data)
   /// @param[in] id buffer ID (should be non-negative)
   /// @return pointer to the buffer
   void* buffer(const int id) const;  
   
   /// @brief size of a single buffer
   /// @return size of a single buffer in bytes
   int bufferSize() const;
   
   /// @brief obtain a buffer to receive data
   /// @details This method return an ID of a free buffer used to
   /// receive the data. If no free buffer is available (i.e. an
   /// overflow situation), a negative value is returned.
   /// @return an ID of the buffer
   int getBufferToFill() const;
   
   /// @brief get filled buffers for a matching channel + beam
   /// @details This method returns the first available set of
   /// completely filled buffers corresponding to the same channel
   /// and beam. The calling thread is blocked until a suitable set
   /// is available for correlation.
   BufferSet getFilledBuffers() const;
   
   /// @brief release the buffers
   /// @details This method notifies the manager that correlation is
   /// now complete and the data buffers can now be released.
   /// @param[in] ids buffer set to release
   void releaseBuffers(const BufferSet &ids) const;
   
   /// @brief notify that the buffer is ready for correlation
   /// @details This method notifies the manager that the data buffer
   /// has now been filled with information and is ready to be correlated.
   /// This finishes operations with this buffer in the I/O thread.
   /// @param[in] id buffer ID (should be non-negative)
   void bufferFilled(const int id) const;
   
   /// @brief get the number of samples
   /// @details This number is hard coded (defined by the data communication
   /// protocol). It is handy to have it defined in a single place and 
   /// access via this method.
   /// @return number of samples (each is complex float)   
   static int NumberOfSamples();
protected:
   /// @brief optional index substitution
   /// @details We want to be quite flexible and allow various substitutions of
   /// indices (e.g. call beam an antenna or renumber them). This method modifies
   /// the header in place for this purpose
   /// @param[in] id buffer ID (should be non-negative)
   /// @note it is assumed that this method called from bufferFilled and the appropriate
   /// mutex lock has been obtained.
   void preprocessIndices(const int id) const;

   /// @brief release single buffer after correlation
   /// @details This method is called from releaseBuffers for each individual
   /// buffer id. It is assumed that the exclusive lock on mutex has already 
   /// been acquired.
   /// @param[in] id buffer ID to release
   void releaseOneBuffer(const int id) const;
   
   /// @brief find a complete set of data 
   /// @details We process all antennas simultaneously (for speed). This method
   /// finds channel/beam numbers which are ready to be correlated
   /// @param[out] index channel/beam pair (output)
   /// @return true if nothing has been found so far, false otherwise
   /// @note The method assumes that a lock has been acquired
   bool findCompleteSet(std::pair<int,int> &index) const;

   /// @brief exception used internally
   struct HelperException : public std::exception {};   
private:
   /// @brief maximum number of buffers supported (fixed at 6*nChan*nBeam)
   int itsNBuf;
   /// @brief size of a single buffer in sizeof(float)
   int itsBufferSize;
   
   /// @brief buffers (stored as one long buffer)
   boost::scoped_array<float> itsBuffer;
   
   /// @brief flags with the buffer status for each buffer
   mutable std::vector<BufferStatus> itsStatus;   
   /// @brief buffer status condition variable
   mutable boost::condition_variable itsStatusCV;
   /// @brief mutex associated with status condition variable
   mutable boost::mutex itsStatusCVMutex;
   /// @brief buffers ID ready for correlation
   /// @details To optimise the look up operation we store IDs for those buffers 
   /// which are ready for correlation into this cube (dimensions are antennas, channels and beams).
   /// All non-negative values stored in this cube correspond to IDs of buffers in the BUF_READY state
   mutable casa::Cube<int> itsReadyBuffers;   
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_BUFFER_MANAGER

