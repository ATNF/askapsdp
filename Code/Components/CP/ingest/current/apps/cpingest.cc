/// @file cpingest.cc
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

// Must be included first
#include "askap_cpingest.h"

// System includes
#include <string>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <unistd.h>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/Log4cxxLogSink.h>
#include <Common/ParameterSet.h>
#include <CommandLineParser.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSinkInterface.h>

// Local package includes
#include "ingestpipeline/IngestPipeline.h"
#include "ingestcontroller/IngestController.h"

// Using
using namespace askap;
using namespace askap::cp;

ASKAP_LOGGER(logger, ".main");

static std::string getNodeName(void)
{
    const int HOST_NAME_MAXLEN = 256;
    char name[HOST_NAME_MAXLEN];
    gethostname(name, HOST_NAME_MAXLEN);
    std::string nodename(name);

    std::string::size_type idx = nodename.find_first_of('.');
    if (idx != std::string::npos) {
        // Extract just the hostname part
        nodename = nodename.substr(0, idx);
    }

    return nodename;
}

void usage(const std::string argv0)
{
        std::cerr << "Usage: " << argv0 << " [options]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "    -inputs <parset file> \t File containing configuration parameters (Standalone mode)" << std::endl;
        std::cerr << "    -brokerURI <brokerURI> \t The URI of the message queue broker (MQ controlled mode)" << std::endl;
        std::cerr << "    -topicURI <topicname>  \t The topic/queue name this program will listen on for commands (MQ controlled mode)" << std::endl;
}

int main(int argc, char *argv[])
{
    try {
        // Now we have to initialize the logger before we use it
        // If a log configuration exists in the current directory then
        // use it, otherwise try to use the programs default one
        std::ifstream config("askap.log_cfg", std::ifstream::in);
        if (config) {
            ASKAPLOG_INIT("askap.log_cfg");
        } else {
            std::ostringstream ss;
            ss << argv[0] << ".log_cfg";
            ASKAPLOG_INIT(ss.str().c_str());
        }

        std::string hostname = getNodeName();
        ASKAPLOG_REMOVECONTEXT("hostname");
        ASKAPLOG_PUTCONTEXT("hostname", hostname.c_str());

        // Ensure that CASA log messages are captured
        casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
        casa::LogSink::globalSink(globalSink);

        // Command line parser
        cmdlineparser::Parser parser;

        // Command line parameter
        cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", "");
        cmdlineparser::FlaggedParameter<std::string> brokerPar("-brokerURI", "");
        cmdlineparser::FlaggedParameter<std::string> topicPar("-topicURI", "");

        // Throw an exception if the parameter is not present
        parser.add(inputsPar, cmdlineparser::Parser::return_default);
        parser.add(brokerPar, cmdlineparser::Parser::return_default);
        parser.add(topicPar, cmdlineparser::Parser::return_default);

        parser.process(argc, const_cast<char**> (argv));

        const std::string parsetFile = inputsPar;
        const std::string brokerURI = brokerPar;
        const std::string topicURI = topicPar;

        // Run the pipeline
        ASKAPLOG_INFO_STR(logger, "ASKAP Central Processor Ingest Pipeline - "
                << ASKAP_PACKAGE_VERSION);

        if (brokerURI == "" && topicURI == "" && parsetFile != "") {
            // Run in standalone mode

            // Create a subset
            LOFAR::ParameterSet parset(parsetFile);
            const LOFAR::ParameterSet subset = parset.makeSubset("cp.ingest.");

            IngestPipeline pipeline(subset);
            pipeline.start();
        } else if (brokerURI != "" && topicURI != "" && parsetFile == ""){
            // Run in MQ controlled mode
            IngestController controller(brokerURI, topicURI);
            controller.run();
        } else {
            usage(argv[0]);
            return 1;
        }

    } catch (const cmdlineparser::XParser& e) {
        usage(argv[0]);
        return 1;
    } catch (const askap::AskapError& e) {
        ASKAPLOG_ERROR_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
        std::cerr << "Askap error in " << argv[0] << ": " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        ASKAPLOG_ERROR_STR(logger, "Unexpected exception in " << argv[0] << ": " << e.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << e.what()
            << std::endl;
        return 1;
    }

    return 0;
}
