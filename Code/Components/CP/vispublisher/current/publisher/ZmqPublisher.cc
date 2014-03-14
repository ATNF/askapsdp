/// @file ZmqPublisher.cc
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
#include "ZmqPublisher.h"

// Include package level header file
#include "askap_vispublisher.h"

// System includes
#include <utility>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include <boost/shared_ptr.hpp>

// Local package includes

ASKAP_LOGGER(logger, ".ZmqPublisher");

using namespace std;
using namespace askap;
using namespace askap::cp::vispublisher;

ZmqPublisher::ZmqPublisher(uint32_t nBeams, uint32_t nPols, uint16_t startPort)
{
    initSockets(nBeams, nPols, startPort);
}

void ZmqPublisher::publish(OutputMessage& outmsg)
{
    // Lookup socket for this beam/pol
    const std::pair<uint32_t, uint32_t> key = make_pair(outmsg.beamId(), outmsg.polId());
    socketmap_t::iterator it = itsSockets.find(key);
    if (it == itsSockets.end()) {
        ASKAPLOG_WARN_STR(logger, "Could not find output socket for beam: " <<
                outmsg.beamId() << ", pol: " << outmsg.polId());
        return;
    }

    // Encode and send message
    zmq::message_t msg(1);
    outmsg.encode(msg);
    it->second->send(msg);
}

void ZmqPublisher::initSockets(uint32_t nBeams, uint32_t nPols, uint16_t startPort)
{
    uint16_t port = startPort;
    for (uint32_t beam = 0; beam < nBeams; ++beam) {
        for (uint32_t pol = 0; pol < nPols; ++pol) {
            const std::pair<uint32_t, uint32_t> key = make_pair(beam, pol);
            
            boost::shared_ptr<zmq::socket_t> value(new zmq::socket_t(itsContext,  ZMQ_PUB));
            stringstream ss;
            ss << "tcp://*:" << port;
            value->bind(ss.str().c_str());
            itsSockets[key] = value;
            ++port;
        }
    }
}
