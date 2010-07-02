/// @file cdeconvolver.cc
///
/// @brief Image deconvolution program
///
/// Performs synthesis imaging from a set of input images.
/// Can run in serial or parallel (MPI) mode.
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

// System includes
#include <stdexcept>
#include <iostream>

// ASKAPsoft includes
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>

ASKAP_LOGGER(logger, ".cdeconvolver");

#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>
#include <CommandLineParser.h>
#include <deconvolution/DeconvolverBase.h>
#include <deconvolution/DeconvolverFactory.h>
#include <deconvolution/DeconvolverHelpers.h>

#include <casa/OS/Timer.h>

using namespace askap;
using namespace askap::synthesis;

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

// Main function
int main(int argc, const char** argv)
{
    try {
        // Ensure that CASA log messages are captured
        casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
        casa::LogSink::globalSink(globalSink);

        casa::Timer timer;
        timer.mark();

        // Put everything in scope to ensure that all destructors are called
        // before the final message
        {
            cmdlineparser::Parser parser; // a command line parser
            // command line parameter
            cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs",
                    "cdeconvolver.in");
            // this parameter is optional
            parser.add(inputsPar, cmdlineparser::Parser::return_default);

            // I hope const_cast is temporary here
            parser.process(argc, argv);

            const std::string parsetFile = inputsPar;

            LOFAR::ParameterSet parset(parsetFile);
            LOFAR::ParameterSet subset(parset.makeSubset("Cdeconvolver."));
 
            std::ostringstream ss;
            ss << argv[0] << ".log_cfg";
            ASKAPLOG_INIT(ss.str().c_str());
            
            std::string hostname = getNodeName();
            ASKAPLOG_REMOVECONTEXT("hostname");
            ASKAPLOG_PUTCONTEXT("hostname", hostname.c_str());

            ASKAPLOG_INFO_STR(logger, "ASKAP image deconvolver " << ASKAP_PACKAGE_VERSION);

            boost::shared_ptr<DeconvolverBase<Float, Complex> > deconvolver(DeconvolverFactory::make(subset));
            deconvolver->deconvolve();

            // Now write the model and residual to disk using the names specified in the 
            // parset. We simply copy the dirty image and then write the array into 
            // the resulting image. 
            DeconvolverHelpers::putArrayToImage(deconvolver->model(), "model", "dirty", subset);
            DeconvolverHelpers::putArrayToImage(deconvolver->dirty(), "residual", "dirty", subset);

       }
        ASKAPLOG_INFO_STR(logger, "Total times - user:   " << timer.user() << " system: " << timer.system()
                              << " real:   " << timer.real());

        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        std::cerr << "Usage: " << argv[0] << " [-inputs parsetFile]"
                      << std::endl;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what()
                      << std::endl;
        exit(1);
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what()
                      << std::endl;
        exit(1);
    }

    return 0;
}

