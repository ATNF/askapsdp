/// @file tActivities.cc
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

// System includes
#include <string>
#include <iostream>

// Ice includes
#include <Ice/Ice.h>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

// Local package includes
#include "activities/AddMetadata.h"
#include "activities/InputPort.h"
#include "activities/OutputPort.h"
#include "streams/Visibilities.h"
#include "streams/Metadata.h"

ASKAP_LOGGER(logger, ".tActivities");

using namespace askap::cp;
using namespace askap::cp::frontend;

const std::string VISSTREAM_NAME = "VisStream0";
const std::string ANNOTATED_VISSTREAM_NAME = "AnnotatedVisStream0";
const std::string METADATASTREAM_NAME = "MetadataStream0";

static void testOne(Ice::CommunicatorPtr ic, Ice::ObjectAdapterPtr adapter)
{
    // Create the activity to test
    AddMetadata activity(ic, adapter);
    activity.attachInputPort(0, METADATASTREAM_NAME);
    activity.attachInputPort(1, VISSTREAM_NAME);
    activity.attachOutputPort(0, ANNOTATED_VISSTREAM_NAME);

    activity.start();

    // Create and configure output port
    askap::cp::OutputPort<Visibilities, IVisStreamPrx> visOutPort(ic);
    visOutPort.attach(VISSTREAM_NAME);
    askap::cp::OutputPort<Metadata, IMetadataStreamPrx> metaOutPort(ic);
    metaOutPort.attach(METADATASTREAM_NAME);

    // Create and configure input port
    askap::cp::InputPort<Visibilities, IVisStream> visInPort(ic, adapter);
    visInPort.attach(ANNOTATED_VISSTREAM_NAME);

    // Send both messages
    Visibilities vis;
    visOutPort.send(vis);
    Metadata md;
    metaOutPort.send(md);

    Visibilities receipt = visInPort.receive();

    // Detach ports from streams
    visOutPort.detach();
    metaOutPort.detach();
    visInPort.detach();
}

int main(int argc, char *argv[])
{
    // Initialize AskapLogging
    ASKAPLOG_INIT("tActivities.log_cfg");

    // Initialise ICE
    Ice::CommunicatorPtr ic;

    try {
        ic = Ice::initialize(argc, argv);

        Ice::ObjectAdapterPtr adapter = ic->createObjectAdapter("tActivitiesAdapter");
        adapter->activate();

        testOne(ic, adapter);

    } catch (const Ice::Exception& e) {
        std::cerr << "Error: " << e << std::endl;
        return 1;
    } catch (const char* msg) {
        std::cerr << "Error: " << msg << std::endl;
        return 1;
    }

    // Shutdown ICE
    ic->shutdown();
    ic->waitForShutdown();

    return 0;
}
