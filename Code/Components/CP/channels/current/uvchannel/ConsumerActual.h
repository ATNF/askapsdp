/// @file ConsumerActual.h
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

#ifndef ASKAP_CP_CHANNELS_CONSUMERACTUAL_H
#define ASKAP_CP_CHANNELS_CONSUMERACTUAL_H

// System includes
#include <string>
#include <vector>
#include <map>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "cms/MessageConsumer.h"
#include "cms/MessageListener.h"
#include "cms/Message.h"

// Local package includes
#include "uvchannel/ConnectionWrapper.h"
#include "uvchannel/IUVChannelListener.h"

namespace askap {
namespace cp {
namespace channels {

    /// @brief This class is the consumer for a given broker/session. This
    /// class encapsulates many ActiveMQ message consumers, one for each
    /// topic.
    /// @ingroup uvchannel
    class ConsumerActual : protected cms::MessageListener {

        public:
            /// Constructor.
            ///
            /// @param[in] brokerURI    the URI used to identify and connect to
            /// the broker.
            /// @param[in] listener     pointer to a class which will recieve
            ///                         a callback every time a VisChunk arrives.
            ConsumerActual(const std::string& brokerURI, IUVChannelListener* listener);

            /// Destructor.
            ~ConsumerActual();

            /// @brief Subscribe this consumer to the specified topic.
            /// @param[in] topic to subscribe to.
            void addSubscription(const std::string& topic);

            /// @brief Unsubscribe this consumer from the specified topic.
            /// @param[in] topic to unsubscribe from.
            void removeSubscription(const std::string& topic);

        protected:

            /// @brief This is an implementation concept. It is the method
            /// via which the client (i.e. this class) is delivered new messages
            /// from the event channel.
            /// @internal
            virtual void onMessage(const cms::Message *message);

        private:

            // Given a topic name, return the MessageConsumer
            boost::shared_ptr<cms::Destination> getDestination(const std::string& topic);

            // No support for assignment
            ConsumerActual& operator=(const ConsumerActual& rhs);

            // No support for copy constructor
            ConsumerActual(const ConsumerActual& src);

            // Channel connection
            ConnectionWrapper itsConnection;

            // Once messages are recieved and converted to a VisChunk, a callback
            // to the object registered here is made
            IUVChannelListener* itsVisListener;

            // Topic map. Maps topic names to MessageConsumers
            std::map<std::string, boost::shared_ptr<cms::MessageConsumer> > itsTopicMap;

            // Buffer for serialising messages. Allows the same memory
            // to be used between invokations of onMessage.
            std::vector<unsigned char> itsBuffer;
    };

};
};
};

#endif
