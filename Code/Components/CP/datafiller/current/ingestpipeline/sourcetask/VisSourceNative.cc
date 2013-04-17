/// @file VisSourceNative.cc
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
#include "VisSourceNative.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/scoped_ptr.hpp"
#include "boost/thread.hpp"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::ingest;

ASKAP_LOGGER(logger, ".VisSourceNative");

VisSourceNative::VisSourceNative(const unsigned int port, const unsigned int bufSize) :
    itsBuffer(bufSize), itsStopRequested(false)
{
    ASKAPLOG_INFO_STR(logger, "VisSourceNative Constructor");
    // Create socket
    itsSockFD = socket(PF_INET, SOCK_DGRAM, 0);
    if (itsSockFD == -1) {
        ASKAPTHROW (std::runtime_error, "Could not create socket. Errno: " << errno);
    }
    
    // Set an 8MB receive buffer to help deal with the bursty nature of the
    // communication
    const int sendsz = 8 * 1024 * 1024;
    int optlen = sizeof(sendsz);
    int err = setsockopt(itsSockFD, SOL_SOCKET, SO_RCVBUF, &sendsz, optlen);
    if (err == -1) {
        ASKAPLOG_WARN_STR(logger, "Setting UDP receive buffer size failed. " <<
                "This may result in dropped datagrams");
    }

    // Setup and bind to port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    err = bind(itsSockFD, (const struct sockaddr *) &addr, sizeof(addr));
    if (err == -1) {
        close(itsSockFD);
        ASKAPTHROW (std::runtime_error, "Could not bind socket. Errno: " << errno);
    }

    // Start the thread
    itsThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&VisSourceNative::run, this)));
    ASKAPLOG_INFO_STR(logger, "VisSourceNative Constructor Exit");
}

VisSourceNative::~VisSourceNative()
{
    ASKAPLOG_INFO_STR(logger, "VisSourceNative Destructor");
    // Signal stopped so now more calls to start_receive() will be made
    itsStopRequested = true;
    close(itsSockFD);

    // Wait for the thread running the io_service to finish
    if (itsThread.get()) {
        itsThread->join();
    }

    // Finally close the socket
    close(itsSockFD);
    ASKAPLOG_INFO_STR(logger, "VisSourceNative Destructor Exit");
}

void VisSourceNative::run(void)
{
    while (!itsStopRequested) {
        //ASKAPLOG_INFO_STR(logger, "VisSourceNative run loop top");
        itsRecvBuffer.reset(new VisDatagram);
        ssize_t size = recv(itsSockFD, itsRecvBuffer.get(), sizeof(VisDatagram), MSG_WAITALL);

        // Will normally expect an error if the socket has been closed, which
        // will occur if the objects destructor is called
        if (size == -1) {
            if (itsStopRequested) {
                break;
            }
            ASKAPLOG_WARN_STR(logger, "Error reading visibilities from UDP socket. " <<
                    "Error Code: " << errno);
            continue;
        }

        if (size != sizeof(VisDatagram)) {
            if (itsStopRequested) {
                break;
            }
            ASKAPLOG_WARN_STR(logger, "Error: Failed to read a full VisDatagram struct");
            continue;
        }
        if (itsRecvBuffer->version != VISPAYLOAD_VERSION) {
            ASKAPLOG_ERROR_STR(logger, "Version mismatch. Expected "
                    << VISPAYLOAD_VERSION
                    << " got " << itsRecvBuffer->version);
            continue;
        }

        // Add a pointer to the message to the back of the circular buffer.
        // Waiters are notified.
        itsBuffer.add(itsRecvBuffer);
    }
}

boost::shared_ptr<VisDatagram> VisSourceNative::next(const long timeout)
{
    return itsBuffer.next(timeout);
}
