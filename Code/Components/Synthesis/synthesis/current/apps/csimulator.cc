/// @file csimulator.cc
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

// Package level header file
#include "askap_synthesis.h"

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/StatReporter.h>
#include <askap/Log4cxxLogSink.h>
#include <askapparallel/AskapParallel.h>
#include <casa/Logging/LogIO.h>
#include <Common/ParameterSet.h>
#include <CommandLineParser.h>

// Local package includes
#include <parallel/SimParallel.h>

ASKAP_LOGGER(logger, ".csimulator");

using namespace std;
using namespace askap;
using namespace askap::synthesis;

// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::askapparallel::AskapParallel comms(argc, argv);

    try {
        StatReporter stats;

        // Ensure that CASA log messages are captured
        casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
        casa::LogSink::globalSink(globalSink);

        {
            cmdlineparser::Parser parser; // a command line parser
            // command line parameter
            cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs",
                    "csimulator.in");
            // this parameter is optional
            parser.add(inputsPar, cmdlineparser::Parser::return_default);

            // I hope const_cast is temporary here
            parser.process(argc, argv);

            // we could have used inputsPar directly in the code below
            const std::string parsetFile = inputsPar;

            LOFAR::ParameterSet parset(parsetFile);
            LOFAR::ParameterSet subset(parset.makeSubset("Csimulator."));

            // We cannot issue log messages until MPI is initialized!
            SimParallel sim(comms, subset);

            ASKAPLOG_INFO_STR(logger, "ASKAP synthesis simulator " << ASKAP_PACKAGE_VERSION);

            if (comms.isMaster()) {
                ASKAPLOG_INFO_STR(logger,  "parset file " << parsetFile);
                ASKAPLOG_INFO_STR(logger,  parset);
            }

            sim.init();
            sim.simulate();
        }
        stats.logSummary();
        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        std::cerr << "Usage: " << argv[0] << " [-inputs parsetFile]" << std::endl;
        exit(1);
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    return 0;
}
