/// @file UVChannelConnection.cc
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

// Include own header file first
#include "UVChannelConnection.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "boost/scoped_ptr.hpp"
#include "activemq/core/ActiveMQConnection.h"
#include "activemq/core/ActiveMQConnectionFactory.h"
#include "cms/Connection.h"
#include "cms/Session.h"
#include "cms/ExceptionListener.h"
#include "cms/Destination.h"
#include "cms/MessageProducer.h"
#include "cms/BytesMessage.h"

ASKAP_LOGGER(logger, ".UVChannelConnection");

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::channels;
using namespace activemq;
using namespace activemq::core;
using namespace cms;

UVChannelConnection::UVChannelConnection(const std::string& brokerURI)
{
    // Create a ConnectionFactory
    boost::scoped_ptr<ActiveMQConnectionFactory> connectionFactory(
        new ActiveMQConnectionFactory(brokerURI));

    try {
        // Create a Connection
        itsConnection.reset(connectionFactory->createConnection());
        ((activemq::core::ActiveMQConnection*)itsConnection.get())->setUseAsyncSend(true);
        itsConnection->start();

        // Create a Session
        itsSession.reset(itsConnection->createSession(cms::Session::AUTO_ACKNOWLEDGE));

        // Create a MessageProducer
        itsProducer.reset(itsSession->createProducer(0));
        itsProducer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

        // Create a BytesMessage
        itsMessage.reset(itsSession->createBytesMessage());
    } catch (const cms::CMSException& e) {
        ASKAPLOG_WARN_STR(logger, "Exception connecting to uv-channel: " << e.getMessage());
        ASKAPTHROW(AskapError, e.getMessage());
    }
}

UVChannelConnection::~UVChannelConnection()
{
    try {
        itsConnection->stop();

        // Cleanup message
        itsMessage.reset();

        // Cleanup producer
        itsProducer->close();
        itsProducer.reset();

        // Cleanup session
        itsSession->close();
        itsSession.reset();

        // Clean up connection
        itsConnection->close();
        itsConnection.reset();
    } catch (const cms::CMSException& e) {
        ASKAPLOG_WARN_STR(logger, "Exception caught in ~UVChannelConnection: "
                << e.getMessage());
    } catch (...) {
        // No exception should escape from destructor
        ASKAPLOG_WARN_STR(logger, "Exception caught in ~UVChannelConnection");
    }
}

void UVChannelConnection::sendByteMessage(const unsigned char* buffer,
        const std::size_t length,
        const std::string& topic)
{
    boost::shared_ptr<cms::Destination> dest = getTopic(topic);
    itsMessage->setBodyBytes(buffer, length);
    itsProducer->send(dest.get(), itsMessage.get());
}

void UVChannelConnection::onException(const cms::CMSException& e)
{
    ASKAPLOG_WARN_STR(logger, "Exception on UVChannel: " << e.getMessage());
}

boost::shared_ptr<cms::Destination> UVChannelConnection::getTopic(const std::string& topic)
{
    boost::shared_ptr<cms::Destination> dest = itsTopicMap[topic];
    if (dest.get() == 0) {
        ASKAPLOG_DEBUG_STR(logger, "Creating destination for topic:" << topic);
        dest.reset(itsSession->createTopic(topic));
    }
    return dest;
}
