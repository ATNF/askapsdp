/// @file PublisherActual.cc
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
#include "PublisherActual.h"

// Include package level header file
#include "askap_channels.h"

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "cms/Session.h"
#include "cms/Destination.h"
#include "cms/MessageProducer.h"
#include "cms/BytesMessage.h"

// Local package includes
#include "uvchannel/ConnectionWrapper.h"

ASKAP_LOGGER(logger, ".PublisherActual");

// Using
using namespace std;
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::channels;
using namespace cms;

/// @brief Publisher wrapper, wrapping a single ActiveMQ MessagePublisher.
PublisherActual::PublisherActual(const std::string& brokerURI) : itsConnection(brokerURI)
{
    ASKAPLOG_DEBUG_STR(logger, "Connecting with URI: " << brokerURI);
    try {
        // Create a MessageProducer
        itsProducer.reset(itsConnection.getSession()->createProducer(0));
        itsProducer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

        // Create a BytesMessage
        itsMessage.reset(itsConnection.getSession()->createBytesMessage());
    } catch (const cms::CMSException& e) {
        ASKAPLOG_WARN_STR(logger, "Exception connecting to uv-channel: " << e.getMessage());
        ASKAPTHROW(AskapError, e.getMessage());
    }
}

PublisherActual::~PublisherActual()
{
    ASKAPLOG_DEBUG_STR(logger, "Disconnecting");
    try {
        // Cleanup message
        itsMessage.reset();

        // Cleanup TopicMap
        itsTopicMap.clear();

        // Cleanup producer
        itsProducer->close();
        itsProducer.reset();
    } catch (const cms::CMSException& e) {
        ASKAPLOG_WARN_STR(logger, "Exception caught in ~PublisherActual: "
                << e.getMessage());
    } catch (...) {
        // No exception should escape from destructor
        ASKAPLOG_WARN_STR(logger, "Exception caught in ~PublisherActual");
    }
}

void PublisherActual::sendByteMessage(const unsigned char* buffer,
        const std::size_t length,
        const std::string& topic)
{
    boost::shared_ptr<cms::Destination> dest = getDestination(topic);
    itsMessage->setBodyBytes(buffer, length);
    itsProducer->send(dest.get(), itsMessage.get());
}

void PublisherActual::sendTextMessage(const std::string& str,
        const std::string& topic)
{
    boost::shared_ptr<cms::Destination> dest = getDestination(topic);
    boost::scoped_ptr<cms::TextMessage> msg(itsConnection.getSession()->createTextMessage(str));
    itsProducer->send(dest.get(), msg.get());
}

boost::shared_ptr<cms::Destination> PublisherActual::getDestination(const std::string& topic)
{
    map< string, boost::shared_ptr<cms::Destination> >::const_iterator it;
    it = itsTopicMap.find(topic);
    if (it == itsTopicMap.end()) {
        ASKAPLOG_DEBUG_STR(logger, "Creating destination for topic: " << topic);
        itsTopicMap[topic].reset(itsConnection.getSession()->createTopic(topic));
    }
    return itsTopicMap[topic];
}
