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

#ifndef ASKAP_SWCORRELATOR_STREAM_CONNECTION
#define ASKAP_SWCORRELATOR_STREAM_CONNECTION

#include <boost/asio.hpp>
#include <swcorrelator/BufferManager.h>

namespace askap {

namespace swcorrelator {

/// @brief Thread which manages a single data stream connection
/// @details This class is initialised with two shared pointers, one to the socket 
/// corresponding to one input data stream and another corresponding to the buffer
/// manager. Each instance (executed as a separate thread), obtains a buffer from 
/// the manager, fills it with new data and de-allocates it. The correlator thread
/// is responsible for further processing when sufficient data are accumulated.
/// @ingroup swcorrelator
struct StreamConnection {

  /// @brief constructor
  /// @details
  /// @param[in] socket shared pointer to the socket corresponding to this connection
  /// @param[in] bm shared pointer to the buffer manager
  StreamConnection(const boost::shared_ptr<boost::asio::ip::tcp::socket> &socket,
                   const boost::shared_ptr<BufferManager> &bm);
  
  /// @brief parallel thread
  /// @details This is the main entry point to the code executed in a parallel thread
  void operator()();
    
private:
  /// @details shared pointer to the socket corresponding connection managed by this instance
  boost::shared_ptr<boost::asio::ip::tcp::socket> itsSocket;
  
  /// @brief buffer manager
  boost::shared_ptr<BufferManager> itsBufferManager;
};


} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_STREAM_CONNECTION

