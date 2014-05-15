/// @file vissubscriber.cc
///
/// @copyright (c) 2014 CSIRO
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

// System includes
#include <iostream>
#include <cassert>
#include <string>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapUtil.h"
#include <zmq.hpp>

using namespace std;
using askap::utility::fromString;
using askap::utility::toString;

struct header_t
{
    uint64_t timestamp;
    uint32_t beam;  // Zero based
    uint32_t pol;   // Polarisation - 0=XX, 1=XY, 2=YX, 3=YY
    uint32_t nChan;
    //.. we ignore the rest because we don't want to print it
};

static void printmsg(std::ostream& os, const zmq::message_t& msg)
{
    assert(msg.size() >= sizeof (header_t));
    const header_t* p = static_cast<const header_t*>(msg.data());
    os << "Received Message - Beam: " << p->beam
        << " Pol: " << p->pol
        << " Time: " << p->timestamp
        << endl;
}

static std::string makeConnectString(const std::string& hostname, int port)
{
    stringstream ss;
    ss << "tcp://" << hostname << ":" << port;
    return ss.str();
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        cerr << "usage: " << argv[0] << " <hostname> <port> <filter1> [filter2]..." << endl;
        return 1;
    }
    const string hostname(argv[1]);
    const int port = fromString<int>(argv[2]);

    zmq::context_t context;
    zmq::socket_t socket (context, ZMQ_SUB);
    const int RECV_HIGH_WATER_MARK = 1; // Only buffer one msg, drop messages if full
    socket.setsockopt(ZMQ_RCVHWM, &RECV_HIGH_WATER_MARK, sizeof (int));
    socket.connect(makeConnectString(hostname, port).c_str());

    // Subscribe to each of the filters passed in
    for (int i = 3; i < argc; ++i) {
        cout << "Subscribing to: " << argv[i] << endl;
        socket.setsockopt(ZMQ_SUBSCRIBE, argv[i], strlen(argv[i]));
    }

    while (true) {
        // Read the identity message
        zmq::message_t identity;
        socket.recv(&identity);

        // Read the payload
        zmq::message_t msg;
        socket.recv(&msg);
        printmsg(cout, msg);
    }

    return 0;
}
