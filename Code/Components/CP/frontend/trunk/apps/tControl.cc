/// @file tControl.cc
///
/// @brief This test ensures the frontend (runtime) can be controlled
/// via the ICE interface.
//
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
#include "streams/SimpleNumber.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::frontend;

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

int main(int argc, char *argv[])
{
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
        Ice::ObjectPrx base = ic->stringToProxy(
                "cpfe_runtime1@cpfe_runtime_test");
        IFrontendPrx frontend = IFrontendPrx::checkedCast(base);
        if (!frontend) {
            std::cerr << "Invalid proxy" << std::endl;
            return 1;
        }

        frontend->startWorkflow(workflow);
        sleep(1);
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

    return 0;
}
