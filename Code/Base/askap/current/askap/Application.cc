/// @file Application.cc
///
/// @copyright (c) 2012 CSIRO
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

// Include own header file first
#include "askap/Application.h"

// Include package level header file
#include "askap_askap.h"

// System includes
#include <string>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <iostream>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Log4cxxLogSink.h"
#include "boost/program_options.hpp"
#include "casa/Logging/LogIO.h"
#include "casa/Logging/LogSinkInterface.h"

ASKAP_LOGGER(logger, ".Application");

// Using/namespace
using namespace askap;
namespace po = boost::program_options;

Application::Application() : itsOptionsDesc("Program Options")
{
    // These are added first so they appear first in the usage message
    itsOptionsDesc.add_options()
    ("help,h", "produce help message")
    ("config,c", po::value<string>(), "configuration parameter set file")
    ("log-config,l", po::value<string>(), "logger configuration file")
    ("inputs", po::value<string>(), "(deprecated) synonym for 'config'. Can also be used with a single hyphen")
    ;
}

Application::~Application()
{
}

int Application::main(int argc, char *argv[])
{
    int status = EXIT_FAILURE;

    try {
        processCmdLineArgs(argc, argv);
        initLogging(argv[0]);
        initConfig();
        status = run(argc, argv);
    } catch (const std::exception& e) {
        if (ASKAPLOG_ISCONFIGURED) {
            ASKAPLOG_FATAL_STR(logger, "Error: " << e.what());
        } else {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        return EXIT_FAILURE;
    }

    return status;
}

LOFAR::ParameterSet Application::config(void) const
{
    return itsParset;
}

bool Application::parameterExists(const std::string& param) const
{
    return itsVarMap.count(param);
}

std::string Application::parameter(const std::string& param) const
{
    if (!parameterExists(param)) {
        ASKAPTHROW(AskapError, "Command line parameter '" << param << "' is not set");
    }
    return itsVarMap[param].as<std::string>();
}

void Application::addParameter(const std::string& keyLong,
                               const std::string& keyShort,
                               const std::string& description,
                               bool hasValue)
{
    const std::string key = buildKey(keyLong, keyShort);
    if (hasValue) {
        itsOptionsDesc.add_options()
        (key.c_str(), po::value<string>(), description.c_str());
    } else {
        itsOptionsDesc.add_options()
        (key.c_str(), description.c_str());
    }
}

void Application::addParameter(const std::string& keyLong,
                               const std::string& keyShort,
                               const std::string& description,
                               const std::string& defaultValue)
{
    const std::string key = buildKey(keyLong, keyShort);
    itsOptionsDesc.add_options()
    (key.c_str(), po::value<string>()->default_value(defaultValue), description.c_str());
}

void Application::processCmdLineArgs(int argc, char *argv[])
{
    //po::store(po::parse_command_line(argc, argv, itsOptionsDesc), itsVarMap);

    // The below allows a single hyphen to be used for the long names. This is
    // to support the historical use of "-inputs"
    po::store(po::command_line_parser(argc, argv).options(itsOptionsDesc).
            style(po::command_line_style::default_style
                | po::command_line_style::allow_long_disguise).run(), itsVarMap);

    po::notify(itsVarMap);

    if (parameterExists("help")) {
        usage();
    }
}

void Application::initLogging(const std::string& argv0)
{
    if (parameterExists("log-config")) {
        // 1: First try the file passed on the command line (fail if it was passed
        // but cannot be accessed.
        const std::string filename = parameter("log-config");
        std::ifstream config(filename.c_str(), std::ifstream::in);
        if (!config) {
            std::cerr << "Error: Failed to open log config file: " << filename << std::endl;
            exit(EXIT_FAILURE);
        }
        ASKAPLOG_INIT(filename.c_str());
    } else {
        std::ifstream config("askap.log_cfg", std::ifstream::in);
        if (config) {
            // 2: Next try the default "askap.log_cfg"
            ASKAPLOG_INIT("askap.log_cfg");
        } else {
            // 3: Finally, look for one where the program resides
            std::ostringstream ss;
            ss << argv0 << ".log_cfg";
            ASKAPLOG_INIT(ss.str().c_str());
        }
    }

    // Set the nodename
    ASKAPLOG_REMOVECONTEXT("hostname");
    ASKAPLOG_PUTCONTEXT("hostname", nodeName().c_str());

    // Ensure that CASA log messages are captured
    casa::LogSinkInterface* globalSink = new askap::Log4cxxLogSink();
    casa::LogSink::globalSink(globalSink);
}

void Application::initConfig()
{
    if (parameterExists("config")) {
        LOFAR::ParameterSet parset(parameter("config"),
                LOFAR::StringUtil::Compare::NOCASE);
        itsParset = parset;
    } else if (parameterExists("inputs")) {
        LOFAR::ParameterSet parset(parameter("inputs"),
                LOFAR::StringUtil::Compare::NOCASE);
        itsParset = parset;
    } else {
        std::cerr << "Error: Configuration file not specified" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Application::usage()
{
    std::cerr << itsOptionsDesc << std::endl;
    exit(EXIT_FAILURE);
}

std::string Application::buildKey(const std::string& keyLong,
                                  const std::string& keyShort)
{
    if (keyLong.size() < 2) {
        ASKAPTHROW(AskapError, "KeyLong must be at least two characters");
    }
    if (keyShort.size() != 1) {
        ASKAPTHROW(AskapError, "KeyShort must be only one character");
    }

    std::ostringstream ss;
    ss << keyLong << "," << keyShort;
    return ss.str();
}

std::string Application::nodeName(void)
{
    const int MAX_HOSTNAME_LEN = 1024;
    char name[MAX_HOSTNAME_LEN];
    name[MAX_HOSTNAME_LEN - 1] = '\0';
    const int error = gethostname(name, MAX_HOSTNAME_LEN - 1);
    if (error) {
        ASKAPTHROW(AskapError, "gethostname() returned error: " << error);
    }

    std::string pname(name);
    std::string::size_type idx = pname.find_first_of('.');
    if (idx != std::string::npos) {
        // Extract just the hostname part
        pname = pname.substr(0, idx);
    }

    return pname;
}
