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
#include <set>
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
#include "publisher/SpdOutputMessage.h"
#include "publisher/InputMessage.h"
#include "publisher/SubsetExtractor.h"
#include "publisher/VisMessageBuilder.h"
#include "publisher/ZmqPublisher.h"

// Using
using namespace std;
using namespace askap;
using namespace askap::cp::vispublisher;
using boost::asio::ip::tcp;

ASKAP_LOGGER(logger, ".PublisherApp");

int PublisherApp::run(int argc, char* argv[])
{
    const uint32_t N_POLS = 4;
    StatReporter stats;
    const LOFAR::ParameterSet subset = config().makeSubset("vispublisher.");
    const uint16_t inPort = subset.getUint16("in.port");
    const uint16_t spdPort = subset.getUint16("spd.port");
    const uint16_t visPort = subset.getUint16("vis.port");

    ASKAPLOG_INFO_STR(logger, "ASKAP Vis Publisher " << ASKAP_PACKAGE_VERSION);
    ASKAPLOG_INFO_STR(logger, "Input Port: " << inPort);
    ASKAPLOG_INFO_STR(logger, "Spd Output Port: " << spdPort);
    ASKAPLOG_INFO_STR(logger, "Vis Output Port: " << visPort);

    // Setup the ZeroMQ publisher objects
    ZmqPublisher spdpub(spdPort);
    ZmqPublisher vispub(visPort);

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

                // Publish SPD data
                const vector<uint32_t> beamvector(inMsg.beam());
                const set<uint32_t> beamset(beamvector.begin(), beamvector.end());
                for (set<uint32_t>::const_iterator beamit = beamset.begin();
                        beamit != beamset.end(); ++beamit) {
                    for (uint32_t pol = 0; pol < N_POLS; ++pol) {
                        SpdOutputMessage outmsg = SubsetExtractor::subset(inMsg, *beamit, pol);
                        ASKAPLOG_DEBUG_STR(logger, "Publishing message for beam " << *beamit
                                << " pol " << pol);
                        spdpub.publish(outmsg);
                    }
                }

                // Publish VIS data
                uint32_t tvChanBegin = 0;
                uint32_t tvChanEnd = inMsg.nChannels() - 1;
                VisOutputMessage outmsg = VisMessageBuilder::build(inMsg,
                        tvChanBegin, tvChanEnd);
                ASKAPLOG_DEBUG_STR(logger, "Publishing Vis message - tvchan: "
                        << tvChanBegin << " - " << tvChanEnd);
                vispub.publish(outmsg);

            } catch (AskapError& e) {
                ASKAPLOG_DEBUG_STR(logger, "Error reading input message: " << e.what()
                        << ", closing input socket");
                socket.close();
            }
        }
    }
    ASKAPLOG_INFO_STR(logger, "Stopping ASKAP Vis Publisher");

    stats.logSummary();
    return 0;
}
