/// @file 
///
/// @brief Thread which manages a single data stream connection
/// @details This class is initialised with two shared pointers, one to the socket 
/// corresponding to one input data stream and another corresponding to the buffer
/// manager. Each instance (executed as a separate thread), obtains a buffer from 
/// the manager, fills it with new data and de-allocates it. The correlator thread
/// is responsible for further processing when sufficient data are accumulated.
/// 
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

#include <swcorrelator/StreamConnection.h>
#include <swcorrelator/BufferHeader.h>
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <boost/thread.hpp>

ASKAP_LOGGER(logger, ".corrworker");

namespace askap {

namespace swcorrelator {

/// @brief constructor
/// @details
/// @param[in] socket shared pointer to the socket corresponding to this connection
/// @param[in] bm shared pointer to the buffer manager
StreamConnection::StreamConnection(const boost::shared_ptr<boost::asio::ip::tcp::socket> &socket,
                 const boost::shared_ptr<BufferManager> &bm) : itsSocket(socket), itsBufferManager(bm) 
{
  ASKAPDEBUGASSERT(socket);
  ASKAPDEBUGASSERT(bm);
}
  
/// @brief parallel thread
/// @details This is the main entry point to the code executed in a parallel thread
void StreamConnection::operator()()
{
  ASKAPLOG_INFO_STR(logger, "Data stream thread started, id="<<boost::this_thread::get_id());
  try {
    ASKAPDEBUGASSERT(itsSocket);
    ASKAPDEBUGASSERT(itsBufferManager); 
    bool haveData = true;
    const int msgSize =sizeof(BufferHeader)/sizeof(int16_t) + 2*BufferManager::NumberOfSamples();
    boost::scoped_array<int16_t> tmpbuf(new int16_t[msgSize]);
    while (haveData) {
       /* 
       boost::this_thread::sleep(boost::posix_time::seconds(1));       
       const uint64_t bat = uint64_t(time(0));
       */
       const int bufId = itsBufferManager->getBufferToFill();       
       if (bufId < 0) {
           ASKAPLOG_FATAL_STR(logger, "Not keeping up - buffer overflow in the data stream thread");
           break;
       }
       ASKAPLOG_DEBUG_STR(logger, "Got bufId="<<bufId<<" from the manager");
       try {
          boost::asio::read(*itsSocket,boost::asio::buffer((void*)tmpbuf.get(), msgSize*sizeof(int16_t)));
          //boost::asio::read(*itsSocket,boost::asio::buffer(itsBufferManager->buffer(bufId), 
          //                itsBufferManager->bufferSize()));
       } catch (const std::exception &ex) {
          haveData = false;
          // release the buffer back without raising a valid flag
          BufferManager::BufferSet bs;
          bs.itsAnt1 = bufId; // others are initialised with -1, no action expected
          itsBufferManager->releaseBuffers(bs);
       }
       if (haveData) {
           int16_t *outbuf = (int16_t*)(itsBufferManager->buffer(bufId));
           int16_t *inbuf = (int16_t*)tmpbuf.get();
           for (size_t i=0; i<sizeof(BufferHeader); i+=sizeof(int16_t)) {
                *(outbuf++) = *(inbuf++);
           }
           std::complex<float>* outbuf2 = (std::complex<float>*)(outbuf);
           for (int i=0; i<BufferManager::NumberOfSamples(); ++i) {
                const int16_t im = *(inbuf++);
                const int16_t rl = *(inbuf++);
                *(outbuf2++) = std::complex<float>(float(rl),float(im));
           }
           // this releases the buffer, but marks it as valid for further processing
           itsBufferManager->bufferFilled(bufId);                 
       }
    }    
    ASKAPLOG_INFO_STR(logger, "Data stream thread (id="<<boost::this_thread::get_id()<<") is finishing (end of the data stream)");
    itsSocket.reset();
    itsBufferManager.reset();
  } catch (const AskapError &ae) {
    ASKAPLOG_FATAL_STR(logger, "Data stream thread (id="<<boost::this_thread::get_id()<<") is about to die: "<<ae.what());
    throw;
  }  
}

} // namespace swcorrelator

} // namespace askap

