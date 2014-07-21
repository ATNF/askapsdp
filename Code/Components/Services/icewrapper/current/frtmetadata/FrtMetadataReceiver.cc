/// @file FrtMetadataReceiver.cc
///
/// @details
/// This file has been converted from Ben's MetadataReceiver and is very similar but
/// just works with a different type (simple map of ints instead of TosMetadata 
/// object). Perhaps we can refactor the class hierarchy to avoid duplication of
/// code. This class is intended to be used in communication with the utility
/// controlling the BETA fringe rotator and/or DRx-based delay tracking.
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
/// @author Max Voronkov <Max.Voronkov@csiro.au>

// Include own header file first
#include "FrtMetadataReceiver.h"

// System includes
#include <string>
#include <map>

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
#include "tosmetadata/TypedValueMapConstMapper.h"

// Using
using namespace askap;
using namespace askap::cp::icewrapper;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;

ASKAP_LOGGER(logger, ".FrtMetadataReceiver");

FrtMetadataReceiver::FrtMetadataReceiver(const std::string& locatorHost,
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

FrtMetadataReceiver::~FrtMetadataReceiver()
{
    if (itsTopicPrx && itsProxy) {
        itsTopicPrx->unsubscribe(itsProxy);
    }
}

void FrtMetadataReceiver::publish(
        const askap::interfaces::TypedValueMap& msg,
        const Ice::Current& /* c */)
{
    TypedValueMapConstMapper mapper(msg);
    // we use the special map item called 'fields_list' to get all fields we're supposed to extract
    // (a bit ugly, but works; may be there is a more elegant way to do the same in ICE)
    const std::vector<casa::String> fields = mapper.getStringSeq("fields_list");
    std::map<std::string, int> convertedMap;
    for (std::vector<casa::String>::const_iterator ci = fields.begin(); ci != fields.end(); ++ci) {
         convertedMap[*ci] = mapper.getInt(*ci);
    }
    
    receive(convertedMap);
}

