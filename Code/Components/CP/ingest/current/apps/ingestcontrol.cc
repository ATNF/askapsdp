/// @file ingestcontrol.cc
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

// System includes
#include <uuid/uuid.h>
#include <unistd.h>
#include <iostream>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "Common/ParameterSet.h" // LOFAR
#include "Common/Exceptions.h"  // LOFAR
#include "CommandLineParser.h"
#include "boost/scoped_ptr.hpp"
#include "ingestctl/IngestControlFascade.h"

// ActiveMQ C++ interface includes
#include "activemq/library/ActiveMQCPP.h"
#include "activemq/core/ActiveMQConnectionFactory.h"

// Using
using namespace askap;
using namespace askap::cp;

/// Print the usage message
/// @param[in] argv0    the program name, typically from argv[0]
void usage(const std::string argv0)
{
        std::cerr << "Usage: " << argv0 << "-brokerURI <URI> -topicURI <URI> -command <command> [options]" << std::endl;
        std::cerr << "  -brokerURI <URI> \tThe URI of the message queue broker" << std::endl;
        std::cerr << "  -topicURI <URI>  \tThe topic/queue name this program will send commands to" << std::endl;
        std::cerr << "  -command <command> \tEither start, abort or state" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -parset <filename> \tFile containing configuration parameters" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const IngestControlFascade::PipelineState& state)
{
    std::string msg;
    switch (state) {
        case IngestControlFascade::IDLE :
            os << "Idle";
            break;
        case IngestControlFascade::STARTING :
            os << "Starting";
            break;
        case IngestControlFascade::RUNNING :
            os << "Running";
            break;
        case IngestControlFascade::SHUTTING_DOWN :
            os << "Shutting down";
            break;
        default:
            os << "Error: Unknown PipelineState";
    }

    return os;
}

int main(int argc, char *argv[])
{
    try {
        // Setup command line parameter
        cmdlineparser::Parser parser;
        cmdlineparser::FlaggedParameter<std::string> brokerPar("-brokerURI");
        cmdlineparser::FlaggedParameter<std::string> topicPar("-topicURI");
        cmdlineparser::FlaggedParameter<std::string> cmdPar("-command");
        cmdlineparser::FlaggedParameter<std::string> parsetPar("-parset", "");

        parser.add(brokerPar, cmdlineparser::Parser::throw_exception);
        parser.add(topicPar, cmdlineparser::Parser::throw_exception);
        parser.add(cmdPar, cmdlineparser::Parser::throw_exception);
        parser.add(parsetPar, cmdlineparser::Parser::return_default);

        parser.process(argc, const_cast<char**> (argv));

        const std::string brokerURI = brokerPar;
        const std::string destURI = topicPar;
        const std::string command = cmdPar;
        const std::string parsetFile = parsetPar;

        // Init the ActiveMQ library
        activemq::library::ActiveMQCPP::initializeLibrary();

        IngestControlFascade ingestControl(brokerURI, destURI);
        if (command == "start") {
            if (parsetFile == "") {
                std::cerr << "Error: Must specify a parset for start command"
                    << std::endl;
                return 1;
            }
            LOFAR::ParameterSet parset(parsetFile);
            ingestControl.start(parset);
        } else if (command == "abort") {
            ingestControl.abort();
        } else if (command == "state") {
            std::cout << ingestControl.getState() << std::endl;
        } else if (command == "shutdown") {
            ingestControl.shutdown();
        } else {
            std::cerr << "Unknown command. Valid commands are "
                << "\"start\", \"abort\", \"state\", and \"shutdown\"" << std::endl;
        }

    } catch (const cmdlineparser::XParser& e) {
        usage(argv[0]);
        return 1;
    } catch (const AskapError& e) {
        std::cerr << "AskapError: " << e.what() << std::endl;
        return 1;
    }

     // Shutdown the ActiveMQ library
     activemq::library::ActiveMQCPP::shutdownLibrary();

    return 0;
}
