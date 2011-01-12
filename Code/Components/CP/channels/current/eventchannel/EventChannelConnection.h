/// @file EventChannelConnection.h
///
/// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_CP_EVENTCHANNEL_EVENTCHANNELCONNECTION_H
#define ASKAP_CP_EVENTCHANNEL_EVENTCHANNELCONNECTION_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "cms/Connection.h"
#include "cms/Session.h"
#include "cms/ExceptionListener.h"

// Local package includes
#include "mq/LibraryWrapper.h"
#include "EventProducer.h"
#include "EventConsumer.h"
#include "EventDestination.h"
#include "EventMessage.h"

namespace askap {
namespace cp {
namespace channels {

    /// @brief The EventChannelConnection class is a singleton object which is
    /// responsible for managing the connection/session to the broker.
    ///
    /// This class implements the event channel using the ActiveMQ CMS
    /// interface. The singleton instance must be first instantiated by calling
    /// the createSingletonInstance() method.
    class EventChannelConnection : protected cms::ExceptionListener {

        public:
            /// @brief Get a reference to the singleton instance of the
            /// EventChannelConnection.
            ///
            /// @return a reference to the singleton instance of the
            /// EventChannelConnection.
            static EventChannelConnection& getSingletonInstance(void);

            /// @brief Create and return the singleton instance of the
            /// EventChannelConnection.
            ///
            /// @param[in] brokerURI    the URI used to identify and connect to
            /// the broker.
            /// @return a reference to the newely created singleton instance
            /// of the EventChannelConnection.
            static EventChannelConnection& createSingletonInstance(const std::string& brokerURI);

            /// @brief Destructor.
            ~EventChannelConnection();

            /// @brief Create an event channel producer object.
            /// @param[in] dest the destination to which the EventProducer is
            ///                 attached.
            /// @return a shared pointer to the EventProducer object.
            EventProducerSharedPtr createEventChannelProducer(EventDestination& dest);

            /// @brief Create an event channel consumer object.
            /// @param[in] dest the destination to which the EventConsumer is
            ///                 attached.
            /// @return a shared pointer to the EventConsumer object.
            EventConsumerSharedPtr createEventChannelConsumer(EventDestination& dest);

            /// @brief Create an event channel destination.
            ///
            /// @param[in] name the name of the topic or queue (i.e. its identifier).
            /// @param[in] type the type of the destination (either topic or queue).
            /// @return a shared pointer to the destination object.
            EventDestinationSharedPtr createEventDestination(const std::string& name,
                    EventDestination::DestinationType type);

            /// @brief Create an event channel event
            /// @return a shared pointer to the EventMessage object.
            EventMessageSharedPtr createEventMessage(void);

        protected:

            /// @brief This is an implementation concept. It is the method
            /// via which the client (i.e. this class) is notified of an
            /// exception condition with the CMS provider.
            /// @internal
            virtual void onException(const cms::CMSException &ex);

        private:

            // Private so only createSingletonInstance can instantiate
            EventChannelConnection(const std::string& brokerURI);

            // No support for assignment
            EventChannelConnection& operator=(const EventChannelConnection& rhs);

            // No support for copy constructor
            EventChannelConnection(const EventChannelConnection& src);

            // Singleton instance of this class
            static EventChannelConnection *itsInstance;

            // ActiveMQ library wrapper (manages the init/shutdown)
            LibraryWrapper mqlib;

            // ActiveMQ Connection
            boost::scoped_ptr<cms::Connection> itsConnection;

            // ActiveMQ Session
            boost::scoped_ptr<cms::Session> itsSession;

    };

};
};
};

#endif
