/// @file udpreceiver.cc
///
/// @copyright (c) 2009 CSIRO
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

#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <apps/Visibilities.h>

using boost::asio::ip::udp;

int main(int argc, char *argv[])
{
    // Create socket
    boost::asio::io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), 1234));

    // Set an 4MB send buffer to help deal with the bursty nature of the
    // communication
    boost::asio::socket_base::receive_buffer_size option(1024 * 1024 * 4);
    boost::system::error_code soerror;
    socket.set_option(option, soerror);
    if (soerror) {
        throw boost::system::system_error(soerror);
    }

    // Create receive buffer
    Visibilities vis;

    // Receive a buffer
    const unsigned int expected = n_baselines * n_beams * n_coarse_chan;
    udp::endpoint remote_endpoint;
    unsigned long timestamp = 0;
    unsigned int i;
    for (i = 0; i < expected; ++i) {
        boost::system::error_code error;
        size_t len = socket.receive_from(boost::asio::buffer(&vis, sizeof(Visibilities)), remote_endpoint, 0, error);
        if (error) {
            throw boost::system::system_error(error);
        }
        if (len != sizeof(Visibilities)) {
            std::cout << "Error: Failed to read a full Visibility struct" << std::endl;
        }

        if (timestamp == 0) {
            timestamp = vis.timestamp;
        }
        if (timestamp > 0 && (vis.timestamp > timestamp)) {
            break;
        }
    }

    double loss = (static_cast<double>(expected) - i) / expected * 100.0;
    std::cout << "Received " << i << " of " << expected << " ( loss " << loss << "% )" << std::endl;

    return 0;
}
