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
#include <mpi.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Log4cxxLogSink.h"
#include "Common/ParameterSet.h"
#include "CommandLineParser.h"
#include "casa/Logging/LogIO.h"
#include "casa/Logging/LogSinkInterface.h"

// Local package includes
#include "ingestpipeline/IngestPipeline.h"

// Using
using namespace askap;
using namespace askap::cp::ingest;

ASKAP_LOGGER(logger, ".main");

static std::string getNodeName(void)
{
    char name[MPI_MAX_PROCESSOR_NAME];
    int resultlen;
    MPI_Get_processor_name(name, &resultlen);
    std::string nodename(name);
    std::string::size_type idx = nodename.find_first_of('.');
    if (idx != std::string::npos) {
        // Extract just the hostname part
        nodename = nodename.substr(0, idx);
    }
    return nodename;
}

static std::string getRank(void)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::ostringstream ss;
    ss << rank;
    return ss.str();
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int error = 0;
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

        // To aid in debugging, the logger needs to know the
        // MPI rank and nodename
        ASKAPLOG_REMOVECONTEXT("mpirank");
        ASKAPLOG_PUTCONTEXT("mpirank", getRank().c_str());
        ASKAPLOG_REMOVECONTEXT("hostname");
        ASKAPLOG_PUTCONTEXT("hostname", getNodeName().c_str());

        ASKAPLOG_INFO_STR(logger, "ASKAP Central Processor Ingest Pipeline - "
                << ASKAP_PACKAGE_VERSION);

        // Ensure that CASA log messages are captured
        casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
        casa::LogSink::globalSink(globalSink);

        // Command line parser
        cmdlineparser::Parser parser;

        // Command line parameter
        cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", "cpingest.in");

        // Throw an exception if the parameter is not present
        parser.add(inputsPar, cmdlineparser::Parser::throw_exception);

        parser.process(argc, const_cast<char**> (argv));

        const std::string parsetFile = inputsPar;

        // Create a subset
        LOFAR::ParameterSet parset(parsetFile);

        // Run the pipeline
        IngestPipeline pipeline(parset);
        pipeline.start();

    } catch (const cmdlineparser::XParser& e) {
        ASKAPLOG_ERROR_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        std::cerr << "Usage: " << argv[0] << " [-inputs parsetFile]" << std::endl;
        error = 1;
    } catch (const askap::AskapError& e) {
        ASKAPLOG_ERROR_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
        std::cerr << "Askap error in " << argv[0] << ": " << e.what() << std::endl;
        error = 1;
    } catch (const std::exception& e) {
        ASKAPLOG_ERROR_STR(logger, "Unexpected exception in " << argv[0] << ": " << e.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << e.what()
            << std::endl;
        error = 1;
    }

    if (error) {
        MPI_Abort(MPI_COMM_WORLD, error);
    } else {
        MPI_Finalize();
    }

    return error;
}
