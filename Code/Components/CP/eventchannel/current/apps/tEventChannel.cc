/// @file tEventChannel.cc
///
/// @description
/// This program snoops the metadata stream being published by the telescope
/// operating system (TOS), decodes the output and writes it to stdout.
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

// ASKAPsoft includes

// Local package includes
#include "eventchannel/EventChannelConnection.h"
#include "eventchannel/EventDestination.h"
#include "eventchannel/EventProducer.h"
#include "eventchannel/EventConsumer.h"
#include "eventchannel/EventMessage.h"

using namespace askap::cp::eventchannel;

// main()
int main(int argc, char *argv[])
{
    // Setup the channel
    EventChannelConnection& conn = EventChannelConnection::createSingletonInstance("tcp://127.0.0.1:61616");

    // Create a destination
    EventDestinationSharedPtr dest = conn.createEventDestination("tEventChannel_topic", EventDestination::TOPIC);

    // Create a producer and consumer
    EventProducerSharedPtr producer = conn.createEventChannelProducer(*dest);
    EventConsumerSharedPtr consumer = conn.createEventChannelConsumer(*dest);

    for (int i = 0; i < 10; ++i) {
        // Send a message
        EventMessageSharedPtr outgoing = conn.createEventMessage();
        const std::string testkey = "test_key";
        const int testval = 123;
        outgoing->setInt(testkey, testval);
        producer->send(*outgoing);

        // Receive the message (waiting for up to 2 seconds)
        EventMessageSharedPtr incoming = consumer->receive(2000);
        if (!incoming) {
            std::cout << "Message NOT received" << std::endl;
            return 1;
        }

        if (!incoming->itemExists(testkey)) {
            std::cout << "Item not in map" << std::endl;
            return 1;
        }

        if (incoming->getInt(testkey) != testval) {
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

    return 0;
}
