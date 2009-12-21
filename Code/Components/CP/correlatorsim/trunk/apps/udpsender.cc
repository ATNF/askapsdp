/// @file udpsender.cc
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
#include <apps/Visibilities.h>
#include <unistd.h>

using boost::asio::ip::udp;

int main(int argc, char *argv[])
{
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
            boost::asio::ip::address::from_string("127.0.0.1"), 1234);

    // Send a pretend inegration
    const unsigned int payloads = n_baselines * n_beams * n_coarse_chan;
    for (unsigned int i = 0; i < payloads + 1; ++i) {
        Visibilities vis;
        if (i < (payloads)) {
            vis.timestamp = 11223344;
        } else {
            vis.timestamp = 11223345;
            std::cout << "Last payload" << std::endl;
        }

        for (unsigned int j = 0; j < n_fine_per_coarse; ++j) {
            vis.vis[j].real = 0;
            vis.vis[j].imag = 0;
        }
        vis.coarseChannel = 1;
        vis.antenna1 = 1;
        vis.antenna2 = 2;
        vis.beam1 = 3;
        vis.beam2 = 4;

        boost::system::error_code error;
        socket.send_to(boost::asio::buffer(&vis, sizeof(Visibilities)), destination, 0, error);
        if (error) {
            throw boost::system::system_error(error);
        }
    }

    std::cout << "Payload size: " << sizeof(Visibilities) << std::endl;

    const double sentSize = static_cast<double>(payloads) * sizeof(Visibilities) / 1024 / 1024 / 1024;
    std::cout << "Sent " << sentSize << " GB" << std::endl;

    return 0;
}
