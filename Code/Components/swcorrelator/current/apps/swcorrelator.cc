/// @file 
///
/// @brief real time software correlator for BETA3 tests
/// @details This application is intended to evolve to become real time software 
/// correlator for BETA3 tests. It takes configuration paramters from the parset file, which
/// allows a flexible control over some parameters which we may need to change during the test 
/// (e.g. beam details, antenna locations, delay fudge factors). 
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// own includes
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <swcorrelator/CorrServer.h>

// casa includes
#include <casa/OS/Timer.h>

// other 3rd party
#include <mwcommon/AskapParallel.h>
#include <Common/ParameterSet.h>
#include <CommandLineParser.h>
#include <signal.h>

// boost includes
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

ASKAP_LOGGER(logger, ".swcorrelator");

void signalHandler(int sig) {
  signal(sig, SIG_DFL);
  askap::swcorrelator::CorrServer::stop();
}

// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::mwcommon::AskapParallel comms(argc, argv);
    
    signal(SIGTERM, &signalHandler);

    try {
       casa::Timer timer;
       timer.mark();
       
       cmdlineparser::Parser parser; // a command line parser
       // command line parameter
       cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs",
                    "swcorrelator.in");
       // this parameter is optional
       parser.add(inputsPar, cmdlineparser::Parser::return_default);

       parser.process(argc, argv);

       const LOFAR::ParameterSet parset(inputsPar);
       const LOFAR::ParameterSet subset(parset.makeSubset("swcorrelator."));
             
       askap::swcorrelator::CorrServer server(subset);
       server.run();       
       
       ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                          << " real:   " << timer.real());
        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        ASKAPLOG_FATAL_STR(logger, "Usage: " << argv[0] << " [-inputs parsetFile]");
        return 1;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        return 1;
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        return 1;
    }

    return 0;
}
