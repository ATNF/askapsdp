/// @file EventChannelConnection.cc
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

// Include own header file first
#include "EventChannelConnection.h"

// Include package level header file
#include <askap_eventchannel.h>

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "boost/scoped_ptr.hpp"
#include "activemq/core/ActiveMQConnectionFactory.h"
#include "activemq/library/ActiveMQCPP.h"
#include "cms/Connection.h"
#include "cms/Session.h"
#include "cms/ExceptionListener.h"
#include "cms/Destination.h"
#include "cms/MessageProducer.h"
#include "cms/MessageConsumer.h"

// Local package includes
#include "EventProducer.h"
#include "EventConsumer.h"
#include "EventDestination.h"
#include "EventMessage.h"

ASKAP_LOGGER(logger, ".EventChannelConnection");

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::eventchannel;
using namespace activemq;
using namespace activemq::core;
using namespace cms;

// Initialize statics
EventChannelConnection* EventChannelConnection::itsInstance = 0;


EventChannelConnection::EventChannelConnection(const std::string& brokerURI)
{
    // This basically assumes only a single ActiveMQ-CPP library user exists
    // (i.e. the event channel package. If other libraries use ActiveMQ then
    // we need to look at a singleton object to encapsulate library
    // initialization and perhaps shutdown.
    activemq::library::ActiveMQCPP::initializeLibrary();

    // Create a ConnectionFactory
    boost::scoped_ptr<ActiveMQConnectionFactory> connectionFactory(
            new ActiveMQConnectionFactory( brokerURI ) );

    try {
        // Create a Connection
        itsConnection.reset(connectionFactory->createConnection());
        itsConnection->start();

        // Create a Session
        itsSession.reset(itsConnection->createSession(cms::Session::AUTO_ACKNOWLEDGE));
    } catch (const cms::CMSException& e) {
        ASKAPTHROW(AskapError, e.getMessage());
    }
}

EventChannelConnection::~EventChannelConnection()
{
    try {
        itsConnection->stop();

        // Cleanup session
        itsSession->close();
        itsSession.reset();

        // Clean up connection
        itsConnection->close();
        itsConnection.reset();

        // Shutdown the ActiveMQ-CPP library
        activemq::library::ActiveMQCPP::shutdownLibrary();
    } catch (...) {
        // No exception should escape from destructor
    }

}

EventChannelConnection& EventChannelConnection::getSingletonInstance(void)
{
    if (!itsInstance) {
        ASKAPTHROW(AskapError, "EventChannelConnection singleton instance not yet created");
    }

    return *itsInstance;
}

EventChannelConnection& EventChannelConnection::createSingletonInstance(const std::string& brokerURI)
{
    if (itsInstance) {
        ASKAPTHROW(AskapError, "EventChannelConnection singleton instance already created");
    }

    itsInstance = new EventChannelConnection(brokerURI);
    return *itsInstance;
}

EventProducerSharedPtr EventChannelConnection::createEventChannelProducer(EventDestination& dest)
{
    cms::MessageProducer* cmsProducer = itsSession->createProducer(dest.getCmsDestination());
    return EventProducerSharedPtr(new EventProducer(*itsSession, cmsProducer));
}

EventConsumerSharedPtr EventChannelConnection::createEventChannelConsumer(EventDestination& dest)
{
    cms::MessageConsumer* cmsConsumer = itsSession->createConsumer(dest.getCmsDestination());
    return EventConsumerSharedPtr(new EventConsumer(*itsSession, cmsConsumer));
}

EventDestinationSharedPtr EventChannelConnection::createEventDestination(const std::string& name,
        EventDestination::DestinationType type)
{

    cms::Destination* cmsDest = 0;

    switch (type) {
        case EventDestination::TOPIC :
            cmsDest = itsSession->createTopic(name);
            break;
        case EventDestination::QUEUE :
            cmsDest = itsSession->createQueue(name);
            break;
        default:
            ASKAPTHROW(AskapError, "Unknown destination type");
            break;
    }

    return EventDestinationSharedPtr(new EventDestination(cmsDest));
}

EventMessageSharedPtr EventChannelConnection::createEventMessage(void)
{
    cms::MapMessage* cmsMessage = itsSession->createMapMessage();
    return EventMessageSharedPtr(new EventMessage(cmsMessage));
}

void EventChannelConnection::onException(const cms::CMSException& e)
{
    ASKAPLOG_WARN_STR(logger, "Exception on EventChannel: " << e.getMessage());
}
