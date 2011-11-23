/// @file 
///
/// @brief main tcp server functionality for the correlator
/// @details This class manages tcp server side which starts a new 
/// receiving thread for each new tcp connection. Each thread receives
/// the data into a buffer from the pool.
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

#ifndef ASKAP_SWCORRELATOR_CORRSERVER_H
#define ASKAP_SWCORRELATOR_CORRSERVER_H

// own includes
#include <swcorrelator/BufferManager.h>
#include <swcorrelator/CorrFiller.h>

// other 3rd party
#include <Common/ParameterSet.h>

// boost includes
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace askap {

namespace swcorrelator {

/// @brief main tcp server functionality for the correlator
/// @details This class manages tcp server side which starts a new 
/// receiving thread for each new tcp connection. Each thread receives
/// the data into a buffer from the pool.
/// @ingroup swcorrelator
class CorrServer {
public:
  /// @brief constructor, attaches the server to the given port
  /// @details Configuration is done via the parset
  /// @param[in] parset parset file with configuration info
  CorrServer(const LOFAR::ParameterSet &parset);
  
  /// @brief run the main loop
  /// @details This method waits for connections and assigns new threads to
  /// manage each connection.
  void run();
  
  /// @brief stop the server
  static void stop();
private:
  
  /// @brief initiate asynchronous accept
  void initAsyncAccept();
  
  /// @brief handler of asynchronous accept
  /// @param[in] e error code
  void asyncAcceptHandler(const boost::system::error_code &e);
   
  /// @brief acceptor
  boost::asio::ip::tcp::acceptor itsAcceptor;
  
  /// @brief I/O threads
  boost::thread_group itsThreads;    

  /// @brief io service
  static boost::asio::io_service theirIOService;
  
  /// @brief flag to stop the server
  static bool theirStopRequested;
  
  /// @brief temporary buffer for a socket to handle asynchronous connection
  boost::shared_ptr<boost::asio::ip::tcp::socket> itsSocketBuf;
  
  /// @brief manager of the buffers
  boost::shared_ptr<BufferManager> itsBufferManager;
  
  /// @brief filler/collater of the result
  boost::shared_ptr<CorrFiller> itsFiller;
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_CORRSERVER_H

