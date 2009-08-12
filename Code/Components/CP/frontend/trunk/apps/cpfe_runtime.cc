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
#include "Ice/Ice.h"
#include "APS/ParameterSet.h"
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/Log4cxxLogSink.h>
#include <APS/ParameterSet.h>
#include <CommandLineParser.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSinkInterface.h>

// Local package includes

// Using
using namespace askap;
using namespace LOFAR::ACC::APS;

ASKAP_LOGGER(logger, ".main");

static Ice::CommunicatorPtr initIce(const ParameterSet& parset)
{
    // Get the initialized property set.
    Ice::PropertiesPtr props = Ice::createProperties();
    ASKAPCHECK(props, "Ice properties creation failed");

    // Get (from parset) and set (into ice props) various configuration
    // parameters    
    std::string tracenet = parset.getString("ice.trace.network", "0");
    props->setProperty("Ice.Trace.Network", tracenet);

    std::string traceprot = parset.getString("ice.trace.protocol", "0");
    props->setProperty("Ice.Trace.Protocol", traceprot);

    std::string locator = parset.getString("ice.locator");
    props->setProperty("Ice.Default.Locator", locator);

    // Initialize a communicator with these properties.
    Ice::InitializationData id;
    id.properties = props;
    return Ice::initialize(id);
}

static Ice::ObjectAdapterPtr createAdapter(const ParameterSet& parset,
        Ice::CommunicatorPtr& ic)
{
    Ice::PropertiesPtr props = ic->getProperties();

    std::string adapterName = parset.getString("ice.adapter.name");
    std::string adapterEndpoint = parset.getString("ice.adapter.endpoints");

    // Need to create props like this (given an adapter name of TestAdapter
    // and an endpoint of tcp)
    // TestAdapter.AdapterId=TestAdapter
    // TestAdapter.Endpoints=tcp
    std::stringstream id;
    id << adapterName << "." << "AdapterId";
    std::stringstream ep;
    ep << adapterName << "." << "Endpoints";

    props->setProperty(id.str(), adapterName);
    props->setProperty(ep.str(), adapterEndpoint);

    Ice::ObjectAdapterPtr adapter = ic->createObjectAdapter(adapterName);

    ASKAPCHECK(ic, "Creation of Ice Adapter failed");

    return adapter;
}

static ParameterSet configure(int argc, char *argv[])
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
    ParameterSet parset(parsetFile);
    return parset.makeSubset("askap.cp.frontend.");
}

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

    // Parse cmdline and get the parameter set
    ParameterSet parset;
    try {
        parset = configure(argc, argv);
    } catch (std::runtime_error& e) {
        ASKAPLOG_FATAL_STR(logger, "Required command line parameters missing");
        std::cerr << "usage: " << argv[0] << " -inputs <pararameter set file>" << std::endl;
        return 1;
    }

    Ice::CommunicatorPtr ic;
    try {
        // Initialise ICE
        ic = initIce(parset);
        ASKAPCHECK(ic, "Initialization of Ice communicator failed");

        Ice::ObjectAdapterPtr adapter = createAdapter(parset, ic);

        adapter->activate();

        // Do something

        adapter->deactivate();
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
