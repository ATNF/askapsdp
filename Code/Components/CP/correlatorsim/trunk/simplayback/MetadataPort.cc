/// @file MetadataPort.cc
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
#include "MetadataPort.h"

// Include package level header file
#include "askap_correlatorsim.h"

// System includes

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"

// Local package includes
#include "cpinterfaces/TypedValues.h"

// Using
using namespace askap::cp;
using namespace askap::interfaces::datapublisher;

ASKAP_LOGGER(logger, ".MetadataPort");

MetadataPort::MetadataPort(const LOFAR::ParameterSet& parset)
    : itsParset(parset)
{
    // Initialise an IceCommunicator from the parset
    Ice::PropertiesPtr props = Ice::createProperties();

    // Make sure that network and protocol tracing are off.
    props->setProperty("Ice.Trace.Network", "0");
    props->setProperty("Ice.Trace.Protocol", "0");

    // Syntax example:
    // IceGrid/Locator:tcp -h localhost -p 4061
    std::ostringstream ss;
    ss << "IceGrid/Locator:tcp -h ";
    ss << itsParset.getString("playback.tossim.ice.locator_host");
    ss << " -p ";
    ss << itsParset.getString("playback.tossim.ice.locator_port");
    std::string locatorParam = ss.str();

    props->setProperty("Ice.Default.Locator", locatorParam);

    // Initialize a communicator with these properties.
    Ice::InitializationData id;
    id.properties = props;
    ASKAPLOG_DEBUG_STR(logger, "Initialising the Ice Communicator");
    itsComm = Ice::initialize(id);
    ASKAPCHECK(itsComm, "Communicator failed to initialise");

    // Get the topic for the metadata stream
    const std::string mdTopicManager =
        itsParset.getString("playback.tossim.icestorm.topicmanager");
    const std::string mdTopic =
        itsParset.getString("playback.tossim.icestorm.topic");
    itsMetadataStream = ITimeTaggedTypedValueMapPublisherPrx::uncheckedCast(
            getProxy(mdTopicManager, mdTopic));
}

MetadataPort::~MetadataPort()
{
}

void MetadataPort::send(const askap::interfaces::TimeTaggedTypedValueMap& payload)
{
    itsMetadataStream->publish(payload);
}

// For a given topic manager and topic, return the proxy to the
// publisher object
Ice::ObjectPrx MetadataPort::getProxy(const std::string& topicManager,
        const std::string& topic)
{
    ASKAPCHECK(itsComm, "Communicator is not initialised");

    Ice::ObjectPrx obj = itsComm->stringToProxy(topicManager);
    IceStorm::TopicManagerPrx manager =
        IceStorm::TopicManagerPrx::checkedCast(obj);
    IceStorm::TopicPrx topicPrx;
    try {
        topicPrx = manager->retrieve(topic);
    } catch (const IceStorm::NoSuchTopic&) {
        try {
            topicPrx = manager->create(topic);
        } catch (const IceStorm::TopicExists&) {
            // Something eles has since created the topic
            topicPrx = manager->retrieve(topic);
        }
    }

    return topicPrx->getPublisher()->ice_twoway();
}

