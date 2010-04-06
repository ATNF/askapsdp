/// @file VisSource.cc
///
/// @copyright (c) 2010 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "VisSource.h"

// Include package level header file
#include <askap_cpingest.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/scoped_ptr.hpp"
#include "boost/thread.hpp"

// Local includes

ASKAP_LOGGER(logger, ".VisSource");

using namespace askap;
using namespace askap::cp;
using boost::asio::ip::udp;

VisSource::VisSource(const unsigned int port, const unsigned int bufSize) :
    itsStopRequested(false)
{
    if (bufSize > 0) {
        itsBuffer.set_capacity(bufSize);
    } else {
        itsBuffer.set_capacity(1);
    }

    // Create socket
    itsSocket.reset(new udp::socket(itsIOService, udp::endpoint(udp::v4(), port)));

    // Set an 16MB receive buffer to help deal with the bursty nature of the
    // communication
    boost::asio::socket_base::receive_buffer_size option(1024 * 1024 * 16);
    boost::system::error_code soerror;
    itsSocket->set_option(option, soerror);
    if (soerror) {
        ASKAPTHROW(AskapError, "Could not set socket options. Error code: "
                << soerror);
    }

    start_receive();

    // Start the thread
    itsThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&VisSource::run, this)));
}

VisSource::~VisSource()
{
    // Signal stopped so now more calls to start_receive() will be made
    itsStopRequested = true;

    // Stop the io_service (non-blocking) and cancel an outstanding requests
    itsIOService.stop();
    itsSocket->cancel();

    // Wait for the thread running the io_service to finish
    if (itsThread.get()) {
        itsThread->join();
    }

    // Finally close the socket
    itsSocket->close();
}

void VisSource::start_receive(void)
{
    itsRecvBuffer.reset(new VisPayload);
    itsSocket->async_receive_from(
            boost::asio::buffer(boost::asio::buffer(itsRecvBuffer.get(), sizeof(VisPayload))),
            itsRemoteEndpoint,
            boost::bind(&VisSource::handle_receive, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

void VisSource::handle_receive(const boost::system::error_code& error,
        std::size_t bytes)
{
    if (!error || error == boost::asio::error::message_size) {
        if (bytes != sizeof(VisPayload)) {
            ASKAPLOG_WARN_STR(logger, "Error: Failed to read a full VisPayload struct");
        }
        if (itsRecvBuffer->version != VISPAYLOAD_VERSION) {
            ASKAPLOG_ERROR_STR(logger, "Version mismatch. Expected "
                    << VISPAYLOAD_VERSION
                    << " got " << itsRecvBuffer->version);
        }

        // Add a pointer to the message to the back of the circular burrer
        boost::mutex::scoped_lock lock(itsMutex);
        itsBuffer.push_back(itsRecvBuffer);
        itsRecvBuffer.reset();

        // Notify any waiters
        lock.unlock();
        itsCondVar.notify_all();
    } else {
        ASKAPLOG_WARN_STR(logger, "Error reading visibilities from UDP socket. Error Code: "
                << error);
    }

    if (!itsStopRequested) {
        start_receive();
    }
}

void VisSource::run(void)
{
    itsIOService.run();    
}

boost::shared_ptr<VisPayload> VisSource::next(void)
{
    boost::mutex::scoped_lock lock(itsMutex);
    while (itsBuffer.empty()) {
        // While this call sleeps/blocks the mutex is released
        itsCondVar.wait(lock);
    }

    // Get the pointer on the front of the circular buffer
    boost::shared_ptr<VisPayload> vis(itsBuffer[0]);
    itsBuffer.pop_front();

    // No need to notify producer. The producer doesn't block because the
    // buffer is a circular buffer.

    return vis;
}
