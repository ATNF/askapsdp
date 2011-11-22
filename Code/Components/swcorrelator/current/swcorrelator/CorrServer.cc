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

// own includes
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <swcorrelator/CorrServer.h>
#include <boost/asio.hpp>

ASKAP_LOGGER(logger, ".swcorrelator");

namespace askap {

namespace swcorrelator {

struct Worker : public IConnection {
  Worker(const boost::shared_ptr<boost::asio::ip::tcp::socket> &socket) : itsSocket(socket) {}
  
  void operator()() {
    ASKAPLOG_INFO_STR(logger, "Thread started to manage connection");
  }
  
  virtual void start() {}
  
private:
  boost::shared_ptr<boost::asio::ip::tcp::socket> itsSocket;
};

bool CorrServer::theirStopRequested = false;

boost::asio::io_service CorrServer::theirIOService;


/// @brief stop the server
void CorrServer::stop()
{
  theirStopRequested = true;
  theirIOService.stop();
}

/// @brief constructor, attaches the server to the given port
/// @details Configuration is done via the parset
/// @param[in] parset parset file with configuration info
CorrServer::CorrServer(const LOFAR::ParameterSet &parset) : itsParset(parset), 
    itsAcceptor(theirIOService)
{
  // setup acceptor
  const int port = itsParset.getInt32("port");
  ASKAPLOG_INFO_STR(logger, "Software correlator will listen port "<<port);
  const int nAnt = itsParset.getInt32("nant",3);
  const int nBeam = itsParset.getInt32("nbeam",1);
  const int nChan = itsParset.getInt32("nchan",1);
  ASKAPLOG_INFO_STR(logger, "Initialise for "<<nAnt<<" antennas and up to "<<nBeam<<" beams and "<<nChan<<" channels(cards)");
  itsBufferManager.reset(new BufferManager(nBeam,nChan));
  
  // initialise tcp endpoint
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
  itsAcceptor.open(endpoint.protocol());
  itsAcceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  itsAcceptor.bind(endpoint);
  itsAcceptor.listen();
  initAsyncAccept();
}
  
/// @brief run the main loop
/// @details This method waits for connections and assigns new threads to
/// manage each connection.
void CorrServer::run()
{
  ASKAPLOG_INFO_STR(logger, "About to run I/O service loop");
  theirIOService.run();
  ASKAPLOG_INFO_STR(logger, "Waiting for all threads to finish");
  itsThreads.join_all();
}

/// @brief initiate asynchronous accept
void CorrServer::initAsyncAccept()
{
  boost::shared_ptr<boost::asio::ip::tcp::socket> socket(new boost::asio::ip::tcp::socket(theirIOService));
  itsConnectionHandlerBuf.reset(new Worker(socket));
  itsAcceptor.async_accept(*socket, boost::bind(&CorrServer::asyncAcceptHandler, this, boost::asio::placeholders::error));
}
  
/// @brief handler of asynchronous accept
/// @param[in] e error code
void CorrServer::asyncAcceptHandler(const boost::system::error_code &e)
{
  if (!e) {
     itsConnectionHandlerBuf->start();
     boost::shared_ptr<Worker> worker = boost::dynamic_pointer_cast<Worker>(itsConnectionHandlerBuf);
     itsThreads.create_thread(*worker);     
  }
  if (!theirStopRequested) {
     initAsyncAccept();
  }
}


} // namespace swcorrelator

} // namespace askap

