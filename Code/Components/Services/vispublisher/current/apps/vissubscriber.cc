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
#include <vector>

// ASKAPsoft includes
#include "askap/AskapUtil.h"
#include <zmq.hpp>

// Local package includes
#include "publisher/VisElement.h"

using namespace std;
using askap::cp::vispublisher::VisElement;
using askap::utility::fromString;
using askap::utility::toString;

struct __attribute__ ((__packed__)) header_t
{
    uint64_t timestamp;
    uint32_t chanBegin;
    uint32_t chanEnd;
    uint32_t nElements;
};

static void printmsg(std::ostream& os, const zmq::message_t& msg)
{
    assert(msg.size() >= sizeof (header_t));
    const header_t* hp = reinterpret_cast<const header_t*>(msg.data());
    os << "Received Message - Time: " << hp->timestamp
        << ", tvChanBegin: " << hp->chanBegin
        << ", tvChanEnd: " << hp->chanEnd
        << ", nElements: " << hp->nElements
        << endl;

    uint32_t nElements = hp->nElements;
    const uint8_t* p = reinterpret_cast<const uint8_t*>(hp + 1);
    for (uint32_t i = 0; i < nElements; ++i) {
        const uint32_t* beam = reinterpret_cast<const uint32_t*>(p);
        p += sizeof (uint32_t);
        const uint32_t* ant1 = reinterpret_cast<const uint32_t*>(p);
        p += sizeof (uint32_t);
        const uint32_t* ant2 = reinterpret_cast<const uint32_t*>(p);
        p += sizeof (uint32_t);
        const uint32_t* pol = reinterpret_cast<const uint32_t*>(p);
        p += sizeof (uint32_t);
        const double* amp = reinterpret_cast<const double*>(p);
        p += sizeof (double);
        const double* phase = reinterpret_cast<const double*>(p);
        p += sizeof (double);
        const double* delay = reinterpret_cast<const double*>(p);
        p += sizeof (double);
        os << "    Beam: " << *beam
            << ", Ant1: " << *ant1
            << ", Ant2: " << *ant2
            << ", Pol: " << *pol
            << ", Amp: " << *amp
            << ", Phase: " << *phase << " deg"
            << ", Delay: "<<  (*delay) * 10.0e9 << " ns "
            << endl;
    }
}

static std::string makeConnectString(const std::string& hostname, int port)
{
    stringstream ss;
    ss << "tcp://" << hostname << ":" << port;
    return ss.str();
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "usage: " << argv[0] << " <hostname> <port>" << endl;
        return 1;
    }
    const string hostname(argv[1]);
    const int port = fromString<int>(argv[2]);

    zmq::context_t context;
    zmq::socket_t socket (context, ZMQ_SUB);
    socket.connect(makeConnectString(hostname, port).c_str());
    socket.setsockopt(ZMQ_SUBSCRIBE, 0, 0);

    while (true) {
        zmq::message_t msg;
        socket.recv(&msg);
        printmsg(cout, msg);
    }

    return 0;
}
