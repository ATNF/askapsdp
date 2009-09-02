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
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "activities/SimpleMath.h"
#include "activities/InputPort.h"
#include "activities/OutputPort.h"
#include "streams/SimpleNumber.h"

ASKAP_LOGGER(logger, ".tActivities");

using namespace askap;
using namespace askap::cp;
using namespace askap::cp::frontend;

const std::string INPUT_A = "InputStreamA";
const std::string INPUT_B = "InputStreamB";
const std::string OUTPUT = "OutputStream";

static void testOne(Ice::CommunicatorPtr ic, Ice::ObjectAdapterPtr adapter)
{
    std::cerr << "Running testOne()" << std::endl;

    // Create the activity to test
    LOFAR::ParameterSet parset;
    SimpleMath activity(ic, adapter, parset);
    activity.attachInputPort(0, INPUT_A);
    activity.attachInputPort(1, INPUT_B);
    activity.attachOutputPort(0, OUTPUT);

    activity.start();

    // Create and configure output port
    askap::cp::OutputPort<SimpleNumber, INumberStreamPrx> outPortA(ic);
    outPortA.attach(INPUT_A);
    askap::cp::OutputPort<SimpleNumber, INumberStreamPrx> outPortB(ic);
    outPortB.attach(INPUT_B);

    // Create and configure input port
    askap::cp::InputPort<SimpleNumber, INumberStream> inPort(ic, adapter);
    inPort.attach(OUTPUT);

    // Send both messages
    SimpleNumber a;
    a.i = 1;
    outPortA.send(a);
    SimpleNumber b;
    b.i = 2;
    outPortB.send(b);

    boost::shared_ptr<SimpleNumber> receipt = inPort.receive();

    if (receipt->i != (a.i + b.i)) {
        ASKAPTHROW(AskapError, "receipt->i != (a.i + b.i)");
    }

    // Detach ports from streams
    outPortA.detach();
    outPortB.detach();
    inPort.detach();

    activity.stop();
}

static void testMulti(Ice::CommunicatorPtr ic, Ice::ObjectAdapterPtr adapter)
{
    std::cerr << "Running testMulti()" << std::endl;

    // Create the activity to test
    LOFAR::ParameterSet parset;
    SimpleMath activity(ic, adapter, parset);
    activity.attachInputPort(0, INPUT_A);
    activity.attachInputPort(1, INPUT_B);
    activity.attachOutputPort(0, OUTPUT);

    activity.start();

    // Create and configure output port
    askap::cp::OutputPort<SimpleNumber, INumberStreamPrx> outPortA(ic);
    outPortA.attach(INPUT_A);
    askap::cp::OutputPort<SimpleNumber, INumberStreamPrx> outPortB(ic);
    outPortB.attach(INPUT_B);

    // Create and configure input port
    askap::cp::InputPort<SimpleNumber, INumberStream> inPort(ic, adapter);
    inPort.attach(OUTPUT);

    for (int i = 0; i < 100; ++i) {
        // Send both messages
        SimpleNumber a;
        a.i = 1;
        outPortA.send(a);
        SimpleNumber b;
        b.i = 2;
        outPortB.send(b);

        boost::shared_ptr<SimpleNumber> receipt = inPort.receive();

        if (receipt->i != (a.i + b.i)) {
            ASKAPTHROW(AskapError, "receipt->i != (a.i + b.i)");
        }
    }

    // Detach ports from streams
    outPortA.detach();
    outPortB.detach();
    inPort.detach();

    activity.stop();
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
        testMulti(ic, adapter);

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
