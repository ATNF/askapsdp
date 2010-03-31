/// @file MetadataBridge.cc
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
#include "MetadataBridge.h"

// Include package level header file
#include "askap_bridges.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"
#include "boost/scoped_ptr.hpp"

// Local package includes
#include "MetadataOutPort.h"
#include "interfaces/CommonTypes.h"
#include "interfaces/TypedValues.h"

// Using
using namespace askap::cp;
using namespace askap::interfaces;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;

ASKAP_LOGGER(logger, ".MetadataBridge");

MetadataBridge::MetadataBridge(const LOFAR::ParameterSet& parset)
    : itsParset(parset.makeSubset("mdbridge."))
{
    // Setup the ActiveMQ topic for output
    const std::string brokerURI = itsParset.getString("activemq.broker_uri");
    const std::string destURI = itsParset.getString("activemq.dest_uri");
    itsOutPort.reset(new MetadataOutPort(brokerURI, destURI));

    // Setup the Ice interface for input
    const std::string locatorHost = itsParset.getString("ice.locator_host");
    const std::string locatorPort = itsParset.getString("ice.locator_port");
    const std::string adapterName = itsParset.getString("ice.adapter_name");
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
    // CPMetadataBridgeAdapter.AdapterId=CPMetadataBridgeAdapter
    // CPMetadataBridgeAdapter.Endpoints=tcp
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
}

MetadataBridge::~MetadataBridge()
{
    ASKAPLOG_INFO_STR(logger, "CP Metadata bridge is shutting down");
    itsOutPort.reset();
}

void MetadataBridge::run(void)
{
    ASKAPDEBUGASSERT(itsComm);

    // Locate and subscribe to the IceStorm topic
    const std::string toicManager = itsParset.getString("icestorm.topicmanager");
    const std::string topicName = itsParset.getString("icestorm.topic");
    const std::string adapterName = itsParset.getString("ice.adapter_name");

    Ice::ObjectPrx obj = itsComm->stringToProxy("IceStorm/TopicManager");
    IceStorm::TopicManagerPrx topicManager = IceStorm::TopicManagerPrx::checkedCast(obj);
    Ice::ObjectAdapterPtr adapter = itsComm->createObjectAdapter(adapterName);
    Ice::ObjectPrx proxy = adapter->addWithUUID(this)->ice_twoway();
    IceStorm::TopicPrx topic;

    ASKAPLOG_INFO_STR(logger, "Subscribing to topic: " << topicName);

    try {
        topic = topicManager->retrieve(topicName);
    } catch (const IceStorm::NoSuchTopic&) {
        std::cout << "Topic not found. Creating..." << std::endl;
        try {
            topic = topicManager->create(topicName);
        } catch (const IceStorm::TopicExists&) {
            // Someone else has already created it
            topic = topicManager->retrieve(topicName);
        }
    }

    IceStorm::QoS qos;
    qos["reliability"] = "ordered";
    topic->subscribeAndGetPublisher(qos, proxy);

    adapter->activate();
    ASKAPLOG_INFO_STR(logger, "CP Metadata bridge is running");
    itsComm->waitForShutdown();
}

void MetadataBridge::publish(const TimeTaggedTypedValueMap& msg,
                        const Ice::Current& c)
{
    ASKAPLOG_INFO_STR(logger, "Got a message");
}
