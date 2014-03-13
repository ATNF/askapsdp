/// @file PublisherApp.cc
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

// Include own header file first
#include "publisher/PublisherApp.h"

// Include package level header file
#include "askap_vispublisher.h"

// System includes
#include <vector>
#include <complex>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "Common/ParameterSet.h"
#include "askap/StatReporter.h"
#include <zmq.hpp>
#include <boost/asio.hpp>

// Local package includes
#include "publisher/OutputMessage.h"

// Using
using namespace std;
using namespace askap;
using namespace askap::cp::vispublisher;
using boost::asio::ip::tcp;

ASKAP_LOGGER(logger, ".PublisherApp");

struct InputVis {
    uint32_t nRow;
    uint32_t nChannel;
    uint32_t nPol;
    uint64_t time;

    double chanWidth;
    std::vector<double> frequency;

    std::vector<uint32_t> antenna1;
    std::vector<uint32_t> antenna2;
    std::vector<uint32_t> beam;

    std::vector< std::complex<float> > visibility;

    std::vector<uint8_t> flag;
};

int PublisherApp::run(int argc, char* argv[])
{
    StatReporter stats;
    const LOFAR::ParameterSet subset = config().makeSubset("vispublisher.");
    const uint32_t port = subset.getUint32("nbeams");

    // Create a socket for the Ingest Pipeline to connect to
    //boost::asio::io_service io_service;
    //const uint16_t port = subset.getUint16("in.port");
    //tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));
    //tcp::socket socket(io_service);
    //acceptor.accept(socket);

    // Read
    //size_t len =

    zmq::context_t context(1);

    zmq::socket_t socket(context, ZMQ_PUB);
    socket.bind("tcp://*:5555");

    zmq::message_t msg(1);
    OutputMessage outmsg;
    outmsg.encode(msg);

    socket.send(msg);

    stats.logSummary();
    return 0;
}
