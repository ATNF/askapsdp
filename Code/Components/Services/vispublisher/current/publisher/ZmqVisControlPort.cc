/// @file ZmqVisControlPort.cc
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
#include "ZmqVisControlPort.h"

// Include package level header file
#include "askap_vispublisher.h"

// System includes
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"

ASKAP_LOGGER(logger, ".ZmqVisControlPort");

using namespace std;
using namespace askap;
using namespace askap::cp::vispublisher;

ZmqVisControlPort::ZmqVisControlPort(uint16_t port)
    : itsIsSet(false), itsChanBegin(0), itsChanEnd(0),
    itsSocket(itsContext, ZMQ_SUB)
{
    const string endpoint = "tcp://*:" + utility::toString(port);
    itsSocket.bind(endpoint.c_str());
    itsSocket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
}

std::pair<uint32_t, uint32_t> ZmqVisControlPort::tvChan(void)
{
    checkControlSocket();
    return make_pair(itsChanBegin, itsChanEnd);
}

bool ZmqVisControlPort::isTVChanSet(void)
{
    checkControlSocket();
    return itsIsSet;
}

void ZmqVisControlPort::checkControlSocket(void)
{
    int size = 0;
    do {
        zmq::message_t msg;
        try {
            size = itsSocket.recv(&msg, ZMQ_NOBLOCK);
        } catch (zmq::error_t& e) {
            ASKAPLOG_ERROR_STR(logger, "Error while reading from control socket: "
                    << e.what());
            continue;
        }

        if (size > 0) {
            // Interpret message
            const size_t expected = 2 * sizeof (uint32_t);
            if (msg.size() != expected) {
                ASKAPLOG_WARN_STR(logger,
                        "Invalid tvchan control message, expected size "
                        << expected << ", actual " << msg.size());
            } else {
                const uint32_t* newbegin = static_cast<const uint32_t*>(msg.data());
                const uint32_t* newend = newbegin + sizeof (uint32_t);
                itsChanBegin = *newbegin;
                itsChanEnd = *newend;
                itsIsSet = true;
                ASKAPLOG_DEBUG_STR(logger, "New tvChanBegin: " << itsChanBegin
                        << ", tvChanEnd: " << itsChanEnd);
            }
        }
    } while (size > 0);
}
