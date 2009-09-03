/// @file tConfig.cc
///
/// @brief Test a simple end-to-end workflow, ensuring that the
/// activity specific configuration parameters are passed to the activity.
///
/// @details
/// The activity used is the SimpleMath activity, where each activity instance
/// takes two inputs and produces one output. The output is either the sum or
/// product of the two inputs, depending on how to activity is configued.
///
/// The workflow using in this test looks like so:
///
/// NumberStreamA1 ---> +-----------------+
///                     | SimpleMath(add) |--
/// NumberStreamA2 ---> +-----------------+  --   +-----------------+
///                                           ----| SimpleMath(mul) +--> NumberStreamC
/// NumberStreamB1 ---> +-----------------+  --   +-----------------+
///                     | SimpleMath(add) |--
/// NumberStreamB2 ---> +-----------------+
///
/// The idea is to use four numbers as input, say a, b, c & d and expect an
/// output of (a + b) * (c + d)
///
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
#include "Common/ParameterSet.h"
#include "CommandLineParser.h"

// Local package includes
#include "runtime/Frontend.h"
#include "activities/InputPort.h"
#include "activities/OutputPort.h"
#include "streams/SimpleNumber.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::frontend;

// Stream names
static const std::string STREAM_A1 = "NumberStreamA1";
static const std::string STREAM_A2 = "NumberStreamA2";
static const std::string STREAM_B1 = "NumberStreamB1";
static const std::string STREAM_B2 = "NumberStreamB2";
static const std::string STREAM_C = "NumberStreamC";

static LOFAR::ParameterSet getWorkflowSubset(int argc, char *argv[])
{
    // Command line parser
    cmdlineparser::Parser parser;

    // Command line parameter
    cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", "cpfe_runtime.in");

    // Throw an exception if the parameter is not present
    parser.add(inputsPar, cmdlineparser::Parser::throw_exception);

    parser.process(argc, const_cast<char**> (argv));

    const std::string parsetFile = inputsPar;

    // Create a subset
    LOFAR::ParameterSet parset(parsetFile);
    return parset.makeSubset("askap.cp.frontend.workflow.");
}

static WorkflowDesc buildWorkflowDesc(const LOFAR::ParameterSet& parset)
{
    WorkflowDesc workflow;

    LOFAR::ParameterSet::const_iterator it;
    for (it = parset.begin(); it != parset.end(); ++it) {
        const std::string s = it->second;
        workflow[it->first] = s;
    }

    return workflow;
}

int runTest(Ice::CommunicatorPtr ic, Ice::ObjectAdapterPtr adapter)
{
    // Create and configure output ports
    askap::cp::OutputPort<SimpleNumber, INumberStreamPrx> outPortA1(ic);
    outPortA1.attach(STREAM_A1);
    askap::cp::OutputPort<SimpleNumber, INumberStreamPrx> outPortA2(ic);
    outPortA2.attach(STREAM_A2);
    askap::cp::OutputPort<SimpleNumber, INumberStreamPrx> outPortB1(ic);
    outPortB1.attach(STREAM_B1);
    askap::cp::OutputPort<SimpleNumber, INumberStreamPrx> outPortB2(ic);
    outPortB2.attach(STREAM_B2);

    // Create and configure input port
    askap::cp::InputPort<SimpleNumber, INumberStream> inPort(ic, adapter);
    inPort.attach(STREAM_C);

    SimpleNumber a;
    a.i = 1;
    SimpleNumber b;
    b.i = 2;
    SimpleNumber c;
    c.i = 3;
    SimpleNumber d;
    d.i = 4;
    for (int i = 1; i <= 10000; ++i) {
        outPortA1.send(a);
        outPortA2.send(b);
        outPortB1.send(c);
        outPortB2.send(d);

        boost::shared_ptr<SimpleNumber> receipt = inPort.receive();
        const long expected = (a.i + b.i) * (c.i + d.i);
        if (receipt->i != expected) {
            std::cout << "Expected " << expected << " got " << receipt->i << std::endl;
            return 1;
        }

        if ((i % 1000) == 0 && i > 999) {
            std::cout << "Received " << i << " messages OK" << std::endl;
        }
        a.i++;
        b.i++;
        c.i++;
        d.i++;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int status = 1;
    Ice::CommunicatorPtr ic;
    try {
        // Initialize ICE.
        ic = Ice::initialize(argc, argv);

        // Parse cmdline and get the parameter set
        LOFAR::ParameterSet parset = getWorkflowSubset(argc, argv);

        // Convert the parset description of the workflow to something
        // which can be sent via ICE
        WorkflowDesc workflow = buildWorkflowDesc(parset);

        // Obtain the proxy
        Ice::ObjectPrx base = ic->stringToProxy("cpfe_runtime1");
        IFrontendPrx frontend = IFrontendPrx::checkedCast(base);
        if (!frontend) {
            std::cerr << "Invalid proxy" << std::endl;
            return 1;
        }

        // This adapter is simply used for the receive port.
        Ice::ObjectAdapterPtr adapter = ic->createObjectAdapter("tConfigAdapter");
        adapter->activate();

        frontend->startWorkflow(workflow);
        sleep(1);
        status = runTest(ic, adapter);
        frontend->stopWorkflow();
        sleep(1);
        frontend->shutdown();
    } catch (const Ice::Exception& e) {
        std::cerr << "Error: " << e << std::endl;
        return 1;
    } catch (const cmdlineparser::XParser& e) {
        std::cerr << "usage: " << argv[0] << " -inputs <pararameter set file>" << std::endl;
        return 1;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const char* msg) {
        std::cerr << "Error: " << msg << std::endl;
        return 1;
    }

    // Shutdown ICE
    ic->shutdown();
    ic->waitForShutdown();

    return status;
}
