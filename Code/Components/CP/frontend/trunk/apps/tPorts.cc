/// @file tInputPort.cc
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

// Local package includes
#include "activities/InputPort.h"
#include "activities/OutputPort.h"
#include "streams/Visibilities.h"

using askap::cp::frontend::Visibilities;
using askap::cp::frontend::IVisStreamPrx;
using askap::cp::frontend::IVisStream;

const std::string STREAM_NAME = "VisStream0";

static void testOne(Ice::CommunicatorPtr ic, Ice::ObjectAdapterPtr adapter)
{
    // Create and configure output port
    askap::cp::OutputPort<Visibilities, IVisStreamPrx> outPort(ic);
    outPort.attach(STREAM_NAME);

    // Create and configure input port
    askap::cp::InputPort<Visibilities, IVisStream> inPort(ic, adapter);
    inPort.attach(STREAM_NAME);

    // Send a message
    Visibilities vis;
    outPort.send(vis);

    Visibilities receipt = inPort.receive();

    // Detach ports from streams
    outPort.detach();
    inPort.detach();
}

static void testMulti(Ice::CommunicatorPtr ic, Ice::ObjectAdapterPtr adapter)
{
    // Create and configure output port
    askap::cp::OutputPort<Visibilities, IVisStreamPrx> outPort(ic);
    outPort.attach(STREAM_NAME);

    // Create and configure input port
    askap::cp::InputPort<Visibilities, IVisStream> inPort(ic, adapter);
    inPort.attach(STREAM_NAME);

    for (int i = 0; i < 100; ++i) {
        // Send a message
        Visibilities vis;
        outPort.send(vis);

        Visibilities receipt = inPort.receive();
    }

    // Detach ports from streams
    outPort.detach();
    inPort.detach();
}


int main(int argc, char *argv[])
{
    // Initialise ICE
    Ice::CommunicatorPtr ic;

    try {
        ic = Ice::initialize(argc, argv);

        Ice::ObjectAdapterPtr adapter = ic->createObjectAdapter("tPortsAdapter");
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
