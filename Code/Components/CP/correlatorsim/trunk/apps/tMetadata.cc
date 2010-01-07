/// @file tMetadata.cc
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

// Include package level header file

// System includes
#include <string>
#include <iostream>

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"

#include "cpinterfaces/CommonTypes.h"
#include "cpinterfaces/TypedValues.h"

using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;

class MetadataSubscriber : virtual public ITimeTaggedTypedValueMapPublisher{
    public:
        virtual void publish(const TimeTaggedTypedValueMap& msg,
                const Ice::Current& c) {
            std::cout << "Got metadata payload for timestamp: "
                << msg.timestamp << std::endl;
            const TypedValueMap &data = msg.data;

            TypedValueMap::const_iterator it = data.begin();
            while (it != data.end()) {
                std::cout << "\t" << it->first << std::endl;
                ++it;
            }
        }
};

int main(int argc, char *argv[])
{
    Ice::CommunicatorPtr ic = Ice::initialize(argc, argv);

    Ice::ObjectPrx obj = ic->stringToProxy("IceStorm/TopicManager");
    IceStorm::TopicManagerPrx topicManager = IceStorm::TopicManagerPrx::checkedCast(obj);
    Ice::ObjectAdapterPtr adapter = ic->createObjectAdapter("tMetadataAdapter");
    ITimeTaggedTypedValueMapPublisherPtr metadataStream = new MetadataSubscriber;
    Ice::ObjectPrx proxy = adapter->addWithUUID(metadataStream)->ice_twoway();
    IceStorm::TopicPrx topic;

    try {
        topic = topicManager->retrieve("tosmetadata");
    }
    catch (const IceStorm::NoSuchTopic&) {
        std::cout << "Topic not found. Creating..." << std::endl;
        try {
            topic = topicManager->create("tosmetadata");
        } catch (const IceStorm::TopicExists&) {
            // Someone else has already created it
            topic = topicManager->retrieve("tosmetadata");
        }
    }

    IceStorm::QoS qos;
    qos["reliability"] = "ordered";
    topic->subscribeAndGetPublisher(qos, proxy);

    adapter->activate();
    std::cout << "Waiting for messages..." << std::endl;
    ic->waitForShutdown();

    return 0;
}

