/// @file imager.cc
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

// Include package level header file
#include <askap_imager.h>

// System includes
#include <string>
#include <sstream>

// Boost includes
#include <boost/scoped_ptr.hpp>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSinkInterface.h>
#include <askap/Log4cxxLogSink.h>
#include <APS/ParameterSet.h>
#include <CommandLineParser.h>

// Local Package includes
#include "distributedimager/DistributedImager.h"
#include "distributedimager/MPIComms.h"

using namespace askap;
using namespace askap::cp;
using namespace LOFAR::ACC::APS;

ASKAP_LOGGER(logger, ".main");

int main(int argc, char *argv[])
{
    // The MPIcomms class can't have the scope fo the try/catch block. This
    // avoids a master/worker deadlock in the case where an exception is
    // thrown by either the master or worker(s) but not both.
    boost::scoped_ptr<MPIComms> comms_p;

    try {
        // Initialise the logger
        std::ostringstream ss;
        ss << argv[0] << ".log_cfg";
        ASKAPLOG_INIT(ss.str().c_str());

        // Ensure that CASA log messages are captured
        casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
        casa::LogSink::globalSink(globalSink);

        // Command line parser
        cmdlineparser::Parser parser;

        // Command line parameter
        cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", "cimager.in");

        // Throw an exception if the parameter is not present
        parser.add(inputsPar, cmdlineparser::Parser::throw_exception);

        parser.process(argc, const_cast<char**> (argv));

        const std::string parsetFile = inputsPar;

        // Create a subset
        ParameterSet parset(parsetFile);
        ParameterSet subset(parset.makeSubset("Cimager."));

        // Instantiate the comms class
        comms_p.reset(new MPIComms(argc, argv));

        // Instantiate the Distributed Imager
        DistributedImager imager(subset, *comms_p);
        imager.run();
    } catch (const cmdlineparser::XParser& e) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        std::cerr << "Usage: " << argv[0] << " [-inputs parsetFile]" << std::endl;
        return 1;
    } catch (const askap::AskapError& e) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
        std::cerr << "Askap error in " << argv[0] << ": " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << e.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << e.what()
            << std::endl;
        return 1;
    }

    comms_p.reset();

    return 0;
}

