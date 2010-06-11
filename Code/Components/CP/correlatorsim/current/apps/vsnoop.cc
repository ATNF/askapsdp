/// @file vsnoop.cc
///
/// @description
/// This program is used to receive the UDP visibility stream from the
/// correlator (or correlator control computer). It decoded the stream
/// and writes it to stdout.
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
#include "askap_correlatorsim.h"

// System includes
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <unistd.h>

// ASKAPsoft includes
#include "cpcommon/VisDatagram.h"
#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "CommandLineParser.h"

using boost::asio::ip::udp;
using askap::cp::VisDatagram;
using namespace askap::cp;

// Globals
static unsigned int verbose = 0;
static unsigned long count = 0;

// When a SIGTERM is sent to the process this signal handler is called
// in order to report the number of UDP datagrams received
static void termination_handler (int signum)
{
    std::cout << "Received " << count << " datagrams" << std::endl;
    exit(0);
}

// Indexing function for indexing into the VisDatagram vis and
// nSamples arrays
int index(int pol, int chan) {
    return pol + ((N_POL) * chan);
}

// Print the visibilities. Only called when verbose == 2
// The format of the output is:
//
// Visibilities:
//     ch0 [ (0.123, 0.456), (0, 0), (0, 0), (0.123, 0.456) ]
//     ch1 [ (0.123, 0.456), (0, 0), (0, 0), (0.123, 0.456) ]
//     ..
//     ..
static void printAdditional(const VisDatagram& v)
{
    std::cout << "\tVisibilities:" << std::endl;
    for (unsigned int i = 0; i < N_FINE_PER_COARSE; ++i) {
        std::cout << "\t\tch" << i << " [ ";
        for (unsigned int j = 0; j < N_POL; ++j) {
            std::cout << "(" << v.vis[index(j, i)].real <<
                ", " << v.vis[index(j, i)].imag << ")";
            if (j != (N_POL - 1)) {
                std::cout << ", ";
            }
        }
        std::cout << " ] "<< std::endl;
    }
}

// Print the contents of the payload (except the visibilities). This is only
// called when verbose == 2.
// The format of the output is:
//
// Timestamp:  4679919826828364
//    Coarse channel: 0
//    Antenna1:   0
//    Antenna2:   1
//    Beam1:      0
//    Beam2:      0
//    ..
//    ..
static void printPayload(const VisDatagram& v)
{
    std::cout << "Timestamp:\t" << v.timestamp << std::endl;
    std::cout << "\tCoarse channel:\t" << v.coarseChannel << std::endl;
    std::cout << "\tAntenna1:\t" << v.antenna1 << std::endl;
    std::cout << "\tAntenna2:\t" << v.antenna2 << std::endl;
    std::cout << "\tBeam1:\t\t" << v.beam1 << std::endl;
    std::cout << "\tBeam2:\t\t" << v.beam2 << std::endl;
    if (verbose == 2) {
        printAdditional(v);
    }
    std::cout << std::endl;
}

// main()
int main(int argc, char *argv[])
{
    // Parse additional command line parameters
    cmdlineparser::Parser parser;
    cmdlineparser::FlagParameter verbosePar("-v");
    cmdlineparser::FlagParameter veryVerbosePar("-vv");
    cmdlineparser::FlaggedParameter<int> portPar("-p", 3000);
    parser.add(verbosePar, cmdlineparser::Parser::return_default);
    parser.add(veryVerbosePar, cmdlineparser::Parser::return_default);
    parser.add(portPar, cmdlineparser::Parser::return_default);

    try {
        parser.process(argc, const_cast<char**> (argv));
        if (verbosePar.defined()) verbose = 1;
        if (veryVerbosePar.defined()) verbose = 2;
    } catch (const cmdlineparser::XParserExtra&) {
        std::cerr << "usage: " << argv[0] << " [-v] [-vv] [-p <udp port#>]" << std::endl;
        std::cerr << "  -v            \t Verbose, partially display payload" << std::endl;
        std::cerr << "  -vv           \t Very vebose, display netire payload" << std::endl;
        std::cerr << "  -p <udp port#>\t UDP Port number to listen on" << std::endl;
        return 1;
    }

    // Setup a signal handler for SIGTERM
    signal(SIGTERM, termination_handler);

    // Create socket
    boost::asio::io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), portPar));

    // Set an 16MB receive buffer to help deal with the bursty nature of the
    // communication
    boost::asio::socket_base::receive_buffer_size option(1024 * 1024 * 16);
    boost::system::error_code soerror;
    socket.set_option(option, soerror);
    if (soerror) {
        std::cerr << "Warning: Could not set socket option. "
            << " This may result in dropped packets" << std::endl;
    }

    // Create receive buffer
    VisDatagram vis;

    // Receive a buffer
    std::cout << "Listening on UDP port " << portPar << 
        " (press CTRL-C to exit)..." <<  std::endl;
    while (true) {
        udp::endpoint remote_endpoint;
        boost::system::error_code error;
        const size_t len = socket.receive_from(boost::asio::buffer(&vis, sizeof(VisDatagram)), remote_endpoint, 0, error);
        if (error) {
            throw boost::system::system_error(error);
        }
        if (len != sizeof(VisDatagram)) {
            std::cout << "Error: Failed to read a full VisDatagram struct" << std::endl;
            continue;
        }
        if (vis.version != VISPAYLOAD_VERSION) {
            std::cout << "Version mismatch. Expected " << VISPAYLOAD_VERSION
                << " got " << vis.version << std::endl;
            continue;
        }
        if (verbose) {
            printPayload(vis);
        } else {
            if (count % 10000 == 0) {
                std::cout << "Received " << count << " datagrams" << std::endl;
            }
        }
        count++;
    }

    return 0;
}
