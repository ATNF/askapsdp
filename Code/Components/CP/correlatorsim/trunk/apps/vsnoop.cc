/// @file tVisibilities.cc
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

// Include package level header file
#include <askap_correlatorsim.h>

// System includes
#include <iostream>
#include <cstdlib>

// ASKAPsoft includes
#include "cpcommon/VisPayload.h"
#include "boost/asio.hpp"
#include "boost/array.hpp"

using boost::asio::ip::udp;
using askap::cp::VisPayload;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <udp port#>" << std::endl;
        return 1;
    }

    const int port = atoi(argv[1]);
    if (port <= 1) {
        std::cerr << "Invalid port number: " << port << std::endl;
        return 1;
    }

    // Create socket
    boost::asio::io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), port));

    // Set an 16MB receive buffer to help deal with the bursty nature of the
    // communication
    boost::asio::socket_base::receive_buffer_size option(1024 * 1024 * 16);
    boost::system::error_code soerror;
    socket.set_option(option, soerror);
    if (soerror) {
        throw boost::system::system_error(soerror);
    }

    // Create receive buffer
    VisPayload vis;

    // Receive a buffer
    std::cout << "Listening on UDP port " << port << std::endl;
    unsigned long count = 0;
    while (true) {
        udp::endpoint remote_endpoint;
        boost::system::error_code error;
        size_t len = socket.receive_from(boost::asio::buffer(&vis, sizeof(VisPayload)), remote_endpoint, 0, error);
        if (error) {
            throw boost::system::system_error(error);
        }
        if (len != sizeof(VisPayload)) {
            std::cout << "Error: Failed to read a full VisPayload struct" << std::endl;
        }

        if (count % 10000 == 0) {
            std::cout << "Received " << count << std::endl;
        }
        count++;
    }

    return 0;
}
