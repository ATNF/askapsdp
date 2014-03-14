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
#include "publisher/InputMessage.h"
#include "publisher/ZmqPublisher.h"

// Using
using namespace std;
using namespace askap;
using namespace askap::cp::vispublisher;
using boost::asio::ip::tcp;

ASKAP_LOGGER(logger, ".PublisherApp");

OutputMessage PublisherApp::buildOutputMessage(const InputMessage& in,
                                               uint32_t beam, uint32_t pol)
{
    OutputMessage out;
    out.timestamp() = in.timestamp();
    out.beamId() = beam;
    out.polId() = pol;
    out.nChannels() = in.nChannels();
    out.chanWidth() = in.chanWidth();
    out.frequency() = in.frequency();
    out.nBaselines() = 21;
    out.antenna1().resize(21);
    out.antenna2().resize(21);
    out.visibilities().resize(21 * in.nChannels());
    out.flag().resize(21 * in.nChannels());

    return out;
}

int PublisherApp::run(int argc, char* argv[])
{
    const uint32_t N_POLS = 4;
    StatReporter stats;
    const LOFAR::ParameterSet subset = config().makeSubset("vispublisher.");
    const uint32_t nBeams = subset.getUint32("nbeams");
    const uint16_t inPort = subset.getUint16("in.port");
    const uint16_t outPortStart = subset.getUint16("out.portstart");

    // Setup the ZeroMQ publisher object
    ZmqPublisher zmqpub(nBeams, N_POLS, outPortStart);

    // Setup the TCP socket to receive data from the ingest pipeline
    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), inPort));
    tcp::socket socket(io_service);
    while (true) {
        acceptor.accept(socket);
        ASKAPLOG_DEBUG_STR(logger, "Accepted incoming connection from: "
                << socket.remote_endpoint().address());

        while (socket.is_open()) {
            try {
                InputMessage inMsg = InputMessage::build(socket);
                ASKAPLOG_DEBUG_STR(logger, "Received a message");
                for (uint32_t beam = 0; beam < nBeams; ++beam) {
                    for (uint32_t pol = 0; pol < N_POLS; ++pol) {
                        OutputMessage outmsg = buildOutputMessage(inMsg, beam, pol);
                        ASKAPLOG_DEBUG_STR(logger, "Publishing message for beam " << beam
                                << " pol " << pol);
                        zmqpub.publish(outmsg);
                    }
                }

            } catch (AskapError& e) {
                ASKAPLOG_DEBUG_STR(logger, "Error reading input message: " << e.what()
                        << ", closing input socket");
                socket.close();
            }
        }
    }

    stats.logSummary();
    return 0;
}
