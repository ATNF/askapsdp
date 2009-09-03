/// @file cpfe_runtime.cc
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

// Must be included first
#include "askap_cpfrontend.h"

// System includes
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Log4cxxLogSink.h"
#include "casa/Logging/LogIO.h"
#include "casa/Logging/LogSinkInterface.h"
#include "Ice/Ice.h"

// Local package includes
#include "runtime/Runtime.h"

// Using
using namespace askap;
using askap::cp::Runtime;

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

int main(int argc, char *argv[])
{
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

    // ### Logging is now setup, can use logger beyond this point ###

    ASKAPLOG_INFO_STR(logger, "ASKAP Central Processor Frontend Runtime - " << ASKAP_PACKAGE_VERSION);


    // Initialise and start the runtime, run() blocks until
    // the runtime is shutdown (via its ICE interface)
    try {
        Ice::CommunicatorPtr ic = Ice::initialize(argc, argv);
        Runtime rt(ic);
        rt.run();
    } catch (const Ice::Exception& e) {
        ASKAPLOG_FATAL_STR(logger, "Ice exception in: " << argv[0] << ": " << e);
        std::cerr << "Ice exception in " << argv[0] << ": " << e << std::endl;
        return 1;
    } catch (const askap::AskapError& e) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
        std::cerr << "Askap error in " << argv[0] << ": " << e.what() << std::endl;
        return 1;
    } catch (const std::runtime_error& e) {
        ASKAPLOG_FATAL_STR(logger, "runtime_error: " << e.what());
        std::cerr << "runtime_error: " << e.what() << std::endl;
        return 1;
    } catch (const char* msg) {
        ASKAPLOG_FATAL_STR(logger, "Error: " << msg);
        std::cerr << "Error: " << msg << std::endl;
        return 1;
    }

    return 0;
}
