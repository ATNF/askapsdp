/// @file ZmqPublisher.h
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

#ifndef ASKAP_CP_VISPUBLISHER_ZMQPUBLISHER_H
#define ASKAP_CP_VISPUBLISHER_ZMQPUBLISHER_H

// System includes
#include <stdint.h>
#include <string>

// ASKAPsoft includes
#include <zmq.hpp>

// Local package includes
#include "publisher/SpdOutputMessage.h"
#include "publisher/VisOutputMessage.h"

namespace askap {
namespace cp {
namespace vispublisher {

/// @brief Encapsulates the code needed to send instance of SpdOutputMessage
/// to subscribers via ZeroMQ.
class ZmqPublisher {
    public:

        /// @brief Constructor
        ZmqPublisher(uint16_t port);

        /// @brief Publish the SPD output message.
        /// @param[in] outmsg   the SpdOutputMessage to publish. The outmsg is actually
        ///                     not modified (despite the reference being non-const),
        ///                     it is just non-const as the message is not fully
        ///                     encapsulated for reasons of performance.
        void publish(SpdOutputMessage& outmsg);

        /// @brief Publish the Vis output message.
        /// @param[in] outmsg   the VisOutputMessage to publish. The outmsg is actually
        ///                     not modified (despite the reference being non-const),
        ///                     it is just non-const as the message is not fully
        ///                     encapsulated for reasons of performance.
        void publish(VisOutputMessage& outmsg);

    private:
        /// Converts a polarisation index to a string.
        /// 0="XX", 1="XY", 2="YX", 3="YY"
        static std::string polToString(int pol);

        /// ZeroMQ context object
        zmq::context_t itsContext;

        /// ZeroMQ socket object
        zmq::socket_t itsSocket;
};

}
}
}

#endif
