/// @file PublisherActual.h
///
/// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_CP_CHANNELS_PUBLISHERACTUAL_H
#define ASKAP_CP_CHANNELS_PUBLISHERACTUAL_H

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "cms/MessageProducer.h"
#include "cms/BytesMessage.h"

// Local package includes
#include "uvchannel/ConnectionWrapper.h"

namespace askap {
namespace cp {
namespace channels {

    /// @brief This class is the producer for a given broker/session. This
    /// class encapsulates a single ActiveMQ message producer, however it can
    /// publish to any destination on the associated broker.
    /// @ingroup uvchannel
    class PublisherActual {

        public:
            /// Constructor
            /// @param[in] brokerURI    the URI used to identify and connect to
            /// the broker.
            PublisherActual(const std::string& brokerURI);

            /// Destructor.
            ~PublisherActual();

            /// @brief Send a byte message to the broker this connection is
            /// connected to
            ///
            /// @param[in] buffer byte buffer
            /// @param[in] length the length of the buffer array in bytes
            /// @param[in] topic pub/sub topic to send the message to
            void sendByteMessage(const unsigned char* buffer,
                    const std::size_t length,
                    const std::string& topic);

            /// @brief Send a test message to the broker this connection is
            /// connected to
            ///
            /// @param[in] str the message string to send
            /// @param[in] topic pub/sub topic to send the message to
            void sendTextMessage(const std::string& str,
                    const std::string& topic);
        private:

            // Given a topic name, return the destination
            boost::shared_ptr<cms::Destination> getDestination(const std::string& topic);

            // No support for assignment
            PublisherActual& operator=(const PublisherActual& rhs);

            // No support for copy constructor
            PublisherActual(const PublisherActual& src);

            // Channel connection
            ConnectionWrapper itsConnection;

            // ActiveMQ MessageProducer
            boost::scoped_ptr<cms::MessageProducer> itsProducer;

            // ActiveMQ BytesMessage
            boost::scoped_ptr<cms::BytesMessage> itsMessage;

            // Topic map
            std::map<std::string, boost::shared_ptr<cms::Destination> > itsTopicMap;
    };

};
};
};

#endif
