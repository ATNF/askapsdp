/// @file ZmqVisControlPort.h
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

#ifndef ASKAP_CP_VISPUBLISHER_ZMQVISCONTROLPORT_H
#define ASKAP_CP_VISPUBLISHER_ZMQVISCONTROLPORT_H

// System includes
#include <stdint.h>
#include <utility>

// ASKAPsoft includes
#include <zmq.hpp>

namespace askap {
namespace cp {
namespace vispublisher {

/// @brief Encapsulates the code needed to send instance of SpdOutputMessage
/// to subscribers via ZeroMQ.
class ZmqVisControlPort {
    public:

        /// @brief Constructor
        /// @param[in] port the TCP port number to bind the control port to
        ZmqVisControlPort(uint16_t port);

        bool isTVChanSet(void);

        /// Get TVCHAN range
        /// @return a pair, the first item being the first channel and the second
        ///         item the second channel. The channel range is inclusive of
        ///         both the begin and end channel.
        std::pair<uint32_t, uint32_t> tvChan(void);

    private:

        /// Check the control socket for new control messages.
        /// This will consume all messages, essentially emptying the
        /// message queue.
        void checkControlSocket(void);

        /// This is true if a control message has been received to set the
        /// channel range, otherwise false. If this is false, the return
        /// value from tvChan should not be used (though it is safe to call
        /// it and the range returned will be 0-0.
        bool itsIsSet;

        /// The first channel (inclusive) in the channel range
        uint32_t itsChanBegin;

        /// The last channel (inclusive) in the channel range
        uint32_t itsChanEnd;

        /// ZeroMQ context object
        zmq::context_t itsContext;

        /// ZeroMQ socket object
        zmq::socket_t itsSocket;
};

}
}
}

#endif
