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
#include <zmq.hpp>

using namespace std;

struct header_t
{
    uint64_t timestamp;
    uint32_t beam;  // Zero based
    uint32_t pol;   // Polarisation - 0=XX, 1=XY, 2=YX, 3=YY
    uint32_t nChan;
};

static void printmsg(std::ostream& os, const zmq::message_t& msg)
{
    assert(msg.size() >= sizeof (header_t));
    const header_t* p = static_cast<const header_t*>(msg.data());
    os << "Timestamp: " << p->timestamp << endl;
    os << "    Beam: " << p->beam << endl;
    os << "    Pol: " << p->pol << endl;
    os << "    nChan: " << p->nChan << endl;
}

static std::string makeConnectString(const std::string& hostname, int port)
{
    stringstream ss;
    ss << "tcp://" << hostname << ":" << port;
    return ss.str();
}

template <typename T>
static T fromString(const std::string& str)
{
    T val;
    stringstream ss(str);
    ss >> val;
    return val;
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        cerr << "usage: " << argv[0] << " <hostname> <start port> <beam> <pol>" << endl;
        return 1;
    }
    const int nPol = 4;
    const string hostname(argv[1]);
    const int startPort = fromString<int>(argv[2]);
    const int beam = fromString<int>(argv[3]);
    const int pol = fromString<int>(argv[4]);


    // Determine port number
    const int port = startPort + pol + (beam * nPol);

    zmq::context_t context;
    zmq::socket_t socket (context, ZMQ_SUB);
    socket.connect(makeConnectString(hostname, port).c_str());
    socket.setsockopt(ZMQ_SUBSCRIBE, 0, 0); // Subscribe to all messages

    while (true) {
        zmq::message_t msg;
        socket.recv(&msg);
        printmsg(cout, msg);
    }

    return 0;
}
