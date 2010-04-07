/// @file MetadataSource.cc
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
#include "MetadataSource.h"

// Include package level header file
#include <askap_cpingest.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include <boost/shared_ptr.hpp>

// Local includes
#include "iceinterfaces/CommonTypes.h"
#include "iceinterfaces/TypedValues.h"

ASKAP_LOGGER(logger, ".MetadataSource");

using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;

MetadataSource::MetadataSource(const std::string& locatorHost,
        const std::string& locatorPort,
        const std::string& topicManager,
        const std::string& topic,
        const std::string& adapterName,
        const unsigned int bufSize) :
    itsBuffer(bufSize)
{
    configureIce(locatorHost, locatorPort, topicManager, topic, adapterName);
}

void MetadataSource::configureIce(const std::string& locatorHost,
        const std::string& locatorPort,
        const std::string& topicManager,
        const std::string& topic,
        const std::string& adapterName)
{
    Ice::PropertiesPtr props = Ice::createProperties();

    // Make sure that network and protocol tracing are off.
    props->setProperty("Ice.Trace.Network", "0");
    props->setProperty("Ice.Trace.Protocol", "0");

    // Increase maximum message size from 1MB to 128MB
    props->setProperty("Ice.MessageSizeMax", "131072");

    // Syntax example:
    // IceGrid/Locator:tcp -h localhost -p 4061
    std::ostringstream ss;
    ss << "IceGrid/Locator:tcp -h ";
    ss << locatorHost;
    ss << " -p ";
    ss << locatorPort;
    std::string locatorParam = ss.str();

    props->setProperty("Ice.Default.Locator", locatorParam);

    // Create adapter property
    // Syntax example:
    // IngestPipeline1Adapter.AdapterId=IngestPipeline1Adapter
    // IngestPipeline1Adapter.Endpoints=tcp
    std::ostringstream adapterId;
    adapterId << adapterName << ".AdapterId";
    props->setProperty(adapterId.str(), adapterName);

    std::ostringstream endpoints;
    endpoints << adapterName << ".Endpoints";
    props->setProperty(endpoints.str(), "tcp");

    // Initialize a communicator with these properties.
    Ice::InitializationData id;
    id.properties = props;
    itsComm = Ice::initialize(id);
    ASKAPDEBUGASSERT(itsComm);

    // Now subscribe to the topic
    Ice::ObjectPrx obj = itsComm->stringToProxy(topicManager);
    IceStorm::TopicManagerPrx topicManagerPrx = IceStorm::TopicManagerPrx::checkedCast(obj);
    Ice::ObjectAdapterPtr adapter = itsComm->createObjectAdapter(adapterName);
    Ice::ObjectPrx proxy = adapter->addWithUUID(this)->ice_twoway();
    IceStorm::TopicPrx topicPrx;

    ASKAPLOG_DEBUG_STR(logger, "Subscribing to topic: " << topic);

    try {
        topicPrx = topicManagerPrx->retrieve(topic);
    } catch (const IceStorm::NoSuchTopic&) {
        ASKAPLOG_DEBUG_STR(logger, "Topic not found, creating.");
        try {
            topicPrx = topicManagerPrx->create(topic);
        } catch (const IceStorm::TopicExists&) {
            // Someone else has already created it
            topicPrx = topicManagerPrx->retrieve(topic);
        }
    }

    IceStorm::QoS qos;
    qos["reliability"] = "ordered";
    topicPrx->subscribeAndGetPublisher(qos, proxy);

    adapter->activate();
}

MetadataSource::~MetadataSource()
{
    itsComm->shutdown();
    itsComm->waitForShutdown();
}

void MetadataSource::publish(const TimeTaggedTypedValueMap& msg,
        const Ice::Current& c)
{
    // Make a copy of the message on the heap
    boost::shared_ptr<TimeTaggedTypedValueMap>
        metadata(new TimeTaggedTypedValueMap(msg));

    // Add a pointer to the message to the back of the circular buffer.
    // Waiters are notified.
    itsBuffer.add(metadata);
}

// Blocking
boost::shared_ptr<TimeTaggedTypedValueMap> MetadataSource::next(void)
{
    return itsBuffer.next();
}
