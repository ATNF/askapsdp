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
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"

// Local package includes

ASKAP_LOGGER(logger, ".ZmqPublisher");

using namespace std;
using namespace askap;
using namespace askap::cp::vispublisher;

ZmqPublisher::ZmqPublisher(uint16_t port)
    :itsSocket(itsContext, 1)
{
    // Limit the number of buffered messages as we don't want to have the
    // consumer read stale data, rather drop messages if the buffer is full.
    // Need to buffer one "cycle" worth which is 9-beams x 4-pols
    const int SEND_HIGH_WATER_MARK = 9 * 4;
    itsSocket.setsockopt(ZMQ_SNDHWM, &SEND_HIGH_WATER_MARK, sizeof (int));

    stringstream ss;
    ss << "tcp://*:" << port;
    itsSocket.bind(ss.str().c_str());
}

void ZmqPublisher::publish(OutputMessage& outmsg)
{
    // Encode and send the identity (e.g. "0XX")
    stringstream ss;
    ss << outmsg.beamId() << polToString(outmsg.polId());
    const size_t sz = ss.str().size();
    zmq::message_t identity(sz + 1);
    memcpy(identity.data(), ss.str().c_str(), sz);
    char* contents = static_cast<char*>(identity.data());
    contents[sz] = 0; // NULL terminate the string
    itsSocket.send(identity, ZMQ_SNDMORE);

    // Encode and send message
    zmq::message_t msg(1);
    outmsg.encode(msg);
    itsSocket.send(msg);
}

std::string ZmqPublisher::polToString(int pol)
{
    switch (pol) {
        case 0: return "XX";
                break;
        case 1: return "XY";
                break;
        case 2: return "YX";
                break;
        case 3: return "YY";
                break;
        default:
                ASKAPTHROW(AskapError, "Unknown polarisation id: " << pol);
                break;
    }
}
