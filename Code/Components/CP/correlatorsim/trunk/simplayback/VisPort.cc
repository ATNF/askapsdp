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
#include <askap_correlatorsim.h>

// System includes

// ASKAPsoft includes
#include <boost/asio.hpp>
#include <unistd.h>

// Local package includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"

// Using
using namespace askap;
using namespace askap::cp;
using boost::asio::ip::udp;

ASKAP_LOGGER(logger, ".VisPort");

VisPort::VisPort(const LOFAR::ParameterSet& parset)
    : itsParset(parset)
{
}

VisPort::~VisPort()
{
}

void VisPort::send(const std::vector<askap::cp::VisPayload>& payload)
{
    const std::string hostname = "127.0.0.1";
        //itsParset.getString("playback.visibilities.hostname");
    const unsigned int port = itsParset.getUint32("playback.visibilities.port");

    // Create a socket
    boost::asio::io_service io_service;
    udp::socket socket(io_service);
    socket.open(udp::v4());

    // Set an 4MB send buffer to help deal with the bursty nature of the
    // communication
    boost::asio::socket_base::send_buffer_size option(1024 * 1024 * 4);
    boost::system::error_code soerror;
    socket.set_option(option, soerror);
    if (soerror) {
        throw boost::system::system_error(soerror);
    }

    // Get the remote endpoint
    boost::asio::ip::udp::endpoint destination(
            boost::asio::ip::address::from_string(hostname), port);

    // Send each payload in the vector
    for (unsigned int i = 0; i < payload.size(); ++i) {
        boost::system::error_code error;
        socket.send_to(boost::asio::buffer(&payload[i], sizeof(VisPayload)), destination, 0, error);
        if (error) {
            throw boost::system::system_error(error);
        }
    }

}
