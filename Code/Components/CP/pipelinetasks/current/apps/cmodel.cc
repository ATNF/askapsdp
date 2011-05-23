/// @file cmodel.cc
///
/// @copyright (c) 2011 CSIRO
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
#include "askap_pipelinetasks.h"

// System include
#include <string>
#include <fstream>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Log4cxxLogSink.h"
#include "Common/ParameterSet.h"
#include "CommandLineParser.h"

// Casacore
#include "casa/Logging/LogIO.h"
#include "casa/Logging/LogSinkInterface.h"

// Local packages includes
#include "cmodel/MPIBasicComms.h"
#include "cmodel/CModelMaster.h"
#include "cmodel/CModelWorker.h"

// Using
using namespace askap;
using namespace askap::cp::pipelinetasks;

ASKAP_LOGGER(logger, ".cmodel");

// main()
int main(int argc, char *argv[])
{
    // Initialize the logger before we use it. If a log configuraton
    // exists in the current directory then use it, otherwise try to
    // use the programs default one.
    std::ifstream config("askap.log_cfg", std::ifstream::in);

    if (config) {
        ASKAPLOG_INIT("askap.log_cfg");
    } else {
        std::ostringstream ss;
        ss << argv[0] << ".log_cfg";
        ASKAPLOG_INIT(ss.str().c_str());
    }

    // Ensure that CASA log messages are captured
    casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
    casa::LogSink::globalSink(globalSink);

    // Command line parser
    cmdlineparser::Parser parser;

    // Command line parameter
    cmdlineparser::FlaggedParameter<string> inputsPar("-inputs", "cmodel.in");

    // Throw an exception if the parameter is not present
    parser.add(inputsPar, cmdlineparser::Parser::return_default);
    parser.process(argc, const_cast<char**>(argv));

    // Create a parset and subset
    LOFAR::ParameterSet parset(inputsPar);
    LOFAR::ParameterSet subset = parset.makeSubset("Cmodel.");

    // Create the comms instance
    MPIBasicComms comms(argc, argv);

    // Instantiate and run the model creator
    if (comms.getId() == 0) {
        CModelMaster master(subset, comms);
        master.run();
    } else {
        CModelWorker worker(comms);
        worker.run();
    }

    return 0;
}
