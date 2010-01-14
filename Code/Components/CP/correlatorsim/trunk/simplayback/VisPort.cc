/// @file VisPort.cc
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
#include "VisPort.h"

// Include package level header file
#include "askap_correlatorsim.h"

// System includes
#include <vector>

// ASKAPsoft includes
#include "boost/asio.hpp"
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "cpcommon/VisPayload.h"

// Using
using namespace askap;
using namespace askap::cp;
using boost::asio::ip::udp;

ASKAP_LOGGER(logger, ".VisPort");

VisPort::VisPort(const std::string& hostname, const std::string& port)
    : itsSocket(itsIOService)
{
    // Open the socket using UDP protocol
    boost::system::error_code operror;
    itsSocket.open(udp::v4(), operror);
    if (operror) {
        ASKAPTHROW(AskapError, "Socket open() call failed");
    }

    // Set an 8MB send buffer to help deal with the bursty nature of the
    // communication. The operating system may have some upper limit on
    // this number.
    boost::asio::socket_base::send_buffer_size option(1024 * 1024 * 8);
    boost::system::error_code soerror;
    itsSocket.set_option(option, soerror);
    if (soerror) {
        ASKAPLOG_WARN_STR(logger, "Failed to set socket option (send buffer size): "
                << soerror);
    }

    // Query the nameservice
    udp::resolver resolver(itsIOService);
    udp::resolver::query query(udp::v4(), hostname, port);

    // Get the remote endpoint
    udp::endpoint destination = *resolver.resolve(query);

    // Connect - remembing this is a UDP socket, so connect does not really
    // connect. It just means the call to send doesn't need to specify the
    // destination each time.
    boost::system::error_code coerror;
    itsSocket.connect(destination, coerror);
    if (coerror) {
        ASKAPTHROW(AskapError, "Socket connect() call failed");
    }
}

VisPort::~VisPort()
{
    itsSocket.close();
}

void VisPort::send(const askap::cp::VisPayload& payload)
{
    boost::system::error_code error;
    itsSocket.send(boost::asio::buffer(&payload, sizeof(VisPayload)), 0, error);
    if (error) {
        ASKAPLOG_ERROR_STR(logger, "UDP send failed: " << error);
    }
}

void VisPort::send(const std::vector<askap::cp::VisPayload>& payload)
{
    // Send each payload in the vector
    for (unsigned int i = 0; i < payload.size(); ++i) {
        send(payload[i]);
    }
}
