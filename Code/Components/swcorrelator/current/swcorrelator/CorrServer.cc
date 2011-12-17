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
#include <swcorrelator/FillerWorker.h>
#include <swcorrelator/CorrWorker.h>
#include <swcorrelator/CaptureWorker.h>
#include <swcorrelator/StreamConnection.h>
#include <swcorrelator/HeaderPreprocessor.h>
#include <boost/asio.hpp>

ASKAP_LOGGER(logger, ".swcorrelator");

namespace askap {

namespace swcorrelator {


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
CorrServer::CorrServer(const LOFAR::ParameterSet &parset) : itsAcceptor(theirIOService), 
     itsCaptureMode(parset.getBool("capturemode", false))
{
  // setup acceptor
  const int port = parset.getInt32("port");
  ASKAPLOG_INFO_STR(logger, "Software correlator will listen port "<<port);

  if (itsCaptureMode) {
     boost::shared_ptr<HeaderPreprocessor> hdrProc(new HeaderPreprocessor(parset));
     itsBufferManager.reset(new BufferManager(2,16, hdrProc));     
  } else {
     itsFiller.reset(new CorrFiller(parset));
     boost::shared_ptr<HeaderPreprocessor> hdrProc(new HeaderPreprocessor(parset));
     itsBufferManager.reset(new BufferManager(itsFiller->nBeam(),itsFiller->nChan(), hdrProc));
  }
  
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
  ASKAPDEBUGASSERT(itsBufferManager);
  if (!itsCaptureMode) {
      ASKAPDEBUGASSERT(itsFiller);
      ASKAPLOG_INFO_STR(logger, "About to start writing thread");
      itsThreads.create_thread(FillerWorker(itsFiller));
      const int nCorrThreads = itsFiller->nBeam() * itsFiller->nChan();
      ASKAPLOG_INFO_STR(logger, "About to start "<<nCorrThreads<<" correlator thread(s)");
      for (int i = 0; i<nCorrThreads; ++i) {
          itsThreads.create_thread(CorrWorker(itsFiller,itsBufferManager));
      }
  } else {
      ASKAPLOG_INFO_STR(logger, "About to start data dump thread");
      itsThreads.create_thread(CaptureWorker(itsBufferManager));
  }
  
  ASKAPLOG_INFO_STR(logger, "About to run I/O service loop");
  theirIOService.run();
  ASKAPLOG_INFO_STR(logger, "Waiting for all I/O and correlator threads to finish");
  itsThreads.interrupt_all();
  itsThreads.join_all();
  ASKAPLOG_INFO_STR(logger, "Shutting down the filler");
  itsFiller->shutdown();
}

/// @brief initiate asynchronous accept
void CorrServer::initAsyncAccept()
{
  itsSocketBuf.reset(new boost::asio::ip::tcp::socket(theirIOService));
  itsAcceptor.async_accept(*itsSocketBuf, boost::bind(&CorrServer::asyncAcceptHandler, this, boost::asio::placeholders::error));
}
  
/// @brief handler of asynchronous accept
/// @param[in] e error code
void CorrServer::asyncAcceptHandler(const boost::system::error_code &e)
{
  if (!e) {
     itsThreads.create_thread(StreamConnection(itsSocketBuf, itsBufferManager));
     itsSocketBuf.reset();
  }
  if (!theirStopRequested) {
     initAsyncAccept();
  }
}


} // namespace swcorrelator

} // namespace askap

