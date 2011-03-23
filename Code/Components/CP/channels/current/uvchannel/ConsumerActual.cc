/// @file ConsumerActual.cc
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
#include "ConsumerActual.h"

// Include package level header file
#include "askap_channels.h"

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "boost/scoped_ptr.hpp"
#include "Blob/BlobIBufVector.h"
#include "Blob/BlobIStream.h"
#include "cms/Session.h"
#include "cms/Destination.h"
#include "cms/MessageConsumer.h"
#include "cms/BytesMessage.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "uvchannel/ConnectionWrapper.h"

ASKAP_LOGGER(logger, ".ConsumerActual");

// Using
using namespace std;
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::common;
using namespace askap::cp::channels;

/// @brief Consumer wrapper, wrapping a single ActiveMQ MessageConsumer.
ConsumerActual::ConsumerActual(const std::string& brokerURI, IUVChannelListener* listener)
    : itsConnection(brokerURI), itsVisListener(listener)
{
    ASKAPLOG_DEBUG_STR(logger, "Connecting with URI: " << brokerURI);
}

ConsumerActual::~ConsumerActual()
{
    try {
        ASKAPLOG_DEBUG_STR(logger, "Disconnecting");

        // Cleanup TopicMap
        itsTopicMap.clear();
    } catch (const cms::CMSException& e) {
        ASKAPLOG_WARN_STR(logger, "Exception caught in ~ConsumerActual: "
                << e.getMessage());
    } catch (...) {
        // No exception should escape from destructor
        ASKAPLOG_WARN_STR(logger, "Exception caught in ~ConsumerActual");
    }
}

void ConsumerActual::addSubscription(const std::string& topic)
{
    map< string, boost::shared_ptr<cms::MessageConsumer> >::const_iterator it;
    it = itsTopicMap.find(topic);
    if (it == itsTopicMap.end()) {
        ASKAPLOG_DEBUG_STR(logger, "Creating destination and consumer for topic: " << topic);
        boost::scoped_ptr<cms::Destination> dest(itsConnection.getSession()->createTopic(topic));
        cms::MessageConsumer* consumer = itsConnection.getSession()->createConsumer(dest.get());
        consumer->setMessageListener(this);
        itsTopicMap[topic].reset(consumer);
    } else {
        ASKAPTHROW(AskapError, "Topic " << topic << " already subscribed to");
    }
}

void ConsumerActual::removeSubscription(const std::string& topic)
{
    map< string, boost::shared_ptr<cms::MessageConsumer> >::iterator it;
    it = itsTopicMap.find(topic);
    if (it == itsTopicMap.end()) {
        ASKAPTHROW(AskapError, "Topic " << topic << " not subscribed to");
    } else {
        it->second->close();
        it->second.reset();
        itsTopicMap.erase(it);
    }
}

void ConsumerActual::onMessage(const cms::Message *message)
{
    std::string destName;
    const cms::Destination* dest = message->getCMSDestination();
    ASKAPASSERT(dest);

    switch (dest->getDestinationType()) {
        case cms::Destination::TOPIC: 
            {
                const cms::Topic* topic = dynamic_cast<const cms::Topic*>(message->getCMSDestination());
                destName = topic->getTopicName();
                break;
            }
        case cms::Destination::QUEUE: 
            {
                const cms::Queue* queue = dynamic_cast<const cms::Queue*>(message->getCMSDestination());
                destName = queue->getQueueName();
                break;
            }
        case cms::Destination::TEMPORARY_TOPIC: 
            {
                const cms::TemporaryTopic* topic = dynamic_cast<const cms::TemporaryTopic*>(message->getCMSDestination());
                destName = topic->getTopicName();
                break;
            }
        case cms::Destination::TEMPORARY_QUEUE: 
            {
                const cms::TemporaryQueue* queue = dynamic_cast<const cms::TemporaryQueue*>(message->getCMSDestination());
                destName = queue->getQueueName();
                break;
            }
        default:
            ASKAPLOG_WARN_STR(logger, "Message with unknown destination type received on uvchannel");
            break;
    }

    const cms::BytesMessage* bytesMessage =
        dynamic_cast<const cms::BytesMessage*>(message);

    if (bytesMessage && itsVisListener) {
        itsBuffer.resize(bytesMessage->getBodyLength());
        bytesMessage->readBytes(itsBuffer);

        boost::shared_ptr<askap::cp::common::VisChunk> chunk(new VisChunk(0,0,0));
        LOFAR::BlobIBufVector<unsigned char> bv(itsBuffer);
        LOFAR::BlobIStream in(bv);
        int version = in.getStart("VisChunk");
        ASKAPASSERT(version == 1);
        in >> *chunk;
        in.getEnd();

        itsVisListener->onMessage(chunk, destName);
        return;
    }

    const cms::TextMessage* textMessage =
        dynamic_cast<const cms::TextMessage*>(message);
    if (textMessage && itsVisListener) {
        itsVisListener->onEndOfStream(destName);
        return;
    }

    ASKAPLOG_WARN_STR(logger, "Message of unexpected type received on uvchannel");
}
