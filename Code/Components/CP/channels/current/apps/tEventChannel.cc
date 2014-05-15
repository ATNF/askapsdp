/// @file tEventChannel.cc
///
/// @description
/// This program executes a very simple testcase for the central processor
/// event channel.
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

// System includes
#include <iostream>
#include <string>
#include <unistd.h>

// Local package includes
#include "eventchannel/EventChannelConnection.h"
#include "eventchannel/EventDestination.h"
#include "eventchannel/EventProducer.h"
#include "eventchannel/EventConsumer.h"
#include "eventchannel/EventMessage.h"
#include "eventchannel/IEventListener.h"

using namespace askap::cp::channels;

class EventListener : public IEventListener {
    public:
        EventListener() : itsCount(0) { };

        virtual void onMessage(const EventMessageSharedPtr message)
        {
            itsCount++;
        };

        unsigned int getCount(void)
        {
            return itsCount;
        };

    private:
        unsigned int itsCount;
};

// main()
int main(int argc, char *argv[])
{
    const std::string brokerURI = "tcp://127.0.0.1:61616";
    const std::string msgType = "TestMessage";
    const std::string destName = "tEventChannel_topic";
    const std::string testKey = "test_key";
    const unsigned int nMessages = 10;

    // Setup the channel
    EventChannelConnection& conn = EventChannelConnection::createSingletonInstance(brokerURI);

    // Create a destination
    EventDestinationSharedPtr dest = conn.createEventDestination(destName, EventDestination::TOPIC);

    // Create a producer and consumer
    EventProducerSharedPtr producer = conn.createEventChannelProducer(*dest);
    EventConsumerSharedPtr consumer = conn.createEventChannelConsumer(*dest);

    for (unsigned int i = 0; i < nMessages; ++i) {
        // Send a message
        EventMessageSharedPtr outgoing = conn.createEventMessage();
        outgoing->setMessageType(msgType);
        const int testval = i;
        outgoing->setInt(testKey, testval);
        producer->send(*outgoing);

        // Receive the message (waiting for up to 2 seconds)
        EventMessageSharedPtr incoming = consumer->receive(2000);
        if (!incoming) {
            std::cout << "Message NOT received" << std::endl;
            return 1;
        }

        if (incoming->getMessageType() != msgType) {
            std::cout << "Message type is incorrect" << std::endl;
            return 1;
        }

        if (!incoming->itemExists(testKey)) {
            std::cout << "Item not in map" << std::endl;
            return 1;
        }

        if (incoming->getInt(testKey) != testval) {
            std::cout << "Map value incorrect" << std::endl;
            return 1;
        }

        std::cout << "Message received and verified" << std::endl;
    }

    // Wait (1 millisecond) for a message that is never going to come
    EventMessageSharedPtr incoming = consumer->receive(1);
    if (incoming) {
        std::cout << "Received an unexpected message" << std::endl;
    }

    //
    // Now test async message receipt via the EventListener
    //
    EventListener listener;
    consumer->setEventListener(&listener);

    // Send a message
    for (unsigned int i = 0; i < nMessages; ++i) {
        EventMessageSharedPtr outgoing = conn.createEventMessage();
        outgoing->setMessageType(msgType);
        const int testval = i + 10;
        outgoing->setInt(testKey, testval);
        producer->send(*outgoing);
    }

    unsigned int retryCount = 0;
    unsigned int msgCount = 0;
    while (msgCount != nMessages && retryCount < 10) {
        msgCount = listener.getCount();
        sleep(1);
    }

    if (msgCount != nMessages) {
        std::cout << "Listener expected " << nMessages << " messages. Got " << msgCount << std::endl;
        return 1;
    }

    return 0;
}
