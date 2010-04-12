/// @file MetadataReceiver.cc
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
#include "MetadataReceiver.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "boost/shared_ptr.hpp"

// CP Ice interfaces
#include "TypedValues.h"

// Local package includes
#include "iceutils/CommunicatorConfig.h"
#include "iceutils/CommunicatorFactory.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;

ASKAP_LOGGER(logger, ".MetadataReceiver");

MetadataReceiver::MetadataReceiver(const std::string& locatorHost,
        const std::string& locatorPort,
        const std::string& topicManager,
        const std::string& topic,
        const std::string& adapterName)
{
    CommunicatorConfig config(locatorHost, locatorPort);
    config.setAdapter(adapterName, "tcp");
    CommunicatorFactory commFactory;
    Ice::CommunicatorPtr comm = commFactory.createCommunicator(config);

    ASKAPDEBUGASSERT(comm);

    // Now subscribe to the topic
    Ice::ObjectPrx obj = comm->stringToProxy(topicManager);
    IceStorm::TopicManagerPrx topicManagerPrx = IceStorm::TopicManagerPrx::checkedCast(obj);
    Ice::ObjectAdapterPtr adapter = comm->createObjectAdapter(adapterName);
    itsProxy = adapter->addWithUUID(this)->ice_twoway();

    ASKAPLOG_DEBUG_STR(logger, "Subscribing to topic: " << topic);

    try {
        itsTopicPrx = topicManagerPrx->retrieve(topic);
    } catch (const IceStorm::NoSuchTopic&) {
        ASKAPLOG_DEBUG_STR(logger, "Topic not found, creating.");
        try {
            itsTopicPrx = topicManagerPrx->create(topic);
        } catch (const IceStorm::TopicExists&) {
            // Someone else has already created it
            itsTopicPrx = topicManagerPrx->retrieve(topic);
        }
    }

    IceStorm::QoS qos;
    qos["reliability"] = "ordered";
    itsTopicPrx->subscribeAndGetPublisher(qos, itsProxy);

    adapter->activate();
}

MetadataReceiver::~MetadataReceiver()
{
    if (itsTopicPrx && itsProxy) {
        itsTopicPrx->unsubscribe(itsProxy);
    }
}

void MetadataReceiver::publish(
        const askap::interfaces::TimeTaggedTypedValueMap& msg,
        const Ice::Current& c)
{
    receive(msg);
}
