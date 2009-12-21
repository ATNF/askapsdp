/// @file SimPlayback.cc
///
/// @copyright (c) 2009 CSIRO
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
#include "SimPlayback.h"

// Include package level header file
#include <askap_correlatorsim.h>

// System includes
#include <string>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces;
using namespace askap::interfaces::cp;
using namespace askap::interfaces::datapublisher;

ASKAP_LOGGER(logger, ".SimPlayback");

SimPlayback::SimPlayback(const LOFAR::ParameterSet& parset)
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
    ss << itsParset.getString("playback.ice.locator_host");
    ss << " -p ";
    ss << itsParset.getString("playback.ice.locator_port");
    std::string locatorParam = ss.str();

    props->setProperty("Ice.Default.Locator", locatorParam);

    // Initialize a communicator with these properties.
    Ice::InitializationData id;
    id.properties = props;
    ASKAPLOG_DEBUG_STR(logger, "Initialising the Ice Communicator");
    itsComm = Ice::initialize(id);
    ASKAPCHECK(itsComm, "Communicator failed to initialise");
}

SimPlayback::~SimPlayback()
{
}

void SimPlayback::run(void)
{
    ASKAPLOG_DEBUG_STR(logger, "Running...");
    ASKAPCHECK(itsComm, "Communicator is not initialised");

    // Get the topic for the metadata stream
    const std::string mdTopicManager =
        itsParset.getString("playback.metadata.icestorm.topicmanager");
    const std::string mdTopic =
        itsParset.getString("playback.metadata.icestorm.topic");
    itsMetadataStream = ITimeTaggedTypedValueMapPublisherPrx::uncheckedCast(
            getProxy(mdTopicManager, mdTopic));

    // Get the topic for the visibility stream
    const std::string visTopicManager =
        itsParset.getString("playback.visibilities.icestorm.topicmanager");
    const std::string visTopic =
        itsParset.getString("playback.visibilities.icestorm.topic");
    itsVisStream = IVisStreamPrx::uncheckedCast(getProxy(visTopicManager, visTopic));
}

Ice::ObjectPrx SimPlayback::getProxy(const std::string& topicManager,
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
        topicPrx = manager->create(topic);
    }

    // Return the proxy
    return topicPrx->getPublisher()->ice_oneway();
}

