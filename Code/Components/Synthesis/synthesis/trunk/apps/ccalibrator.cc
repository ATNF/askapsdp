/// @file ccalibrator.cc
///
/// @brief Perform calibration and write result in the parset file
/// @details This application performs calibration of a measurement set
/// and writes the solution to an external parset file
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

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askapparallel/AskapParallel.h>
#include <casa/OS/Timer.h>
#include <CommandLineParser.h>
#include <Common/ParameterSet.h>
#include <parallel/CalibratorParallel.h>

ASKAP_LOGGER(logger, ".ccalibrator");

using namespace std;
using namespace askap;
using namespace askap::synthesis;
using namespace cmdlineparser;

// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::mwbase::AskapParallel comms(argc, argv);

    try {
        casa::Timer timer;
        timer.mark();

        {
            cmdlineparser::Parser parser; // a command line parser
            // command line parameter
            cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs",
                    "ccalibrator.in");
            // this parameter is optional
            parser.add(inputsPar, cmdlineparser::Parser::return_default);

            // I hope const_cast is temporary here
            parser.process(argc, const_cast<char**>(argv));

            LOFAR::ParameterSet parset(inputsPar);
            LOFAR::ParameterSet subset(parset.makeSubset("Ccalibrator."));

            CalibratorParallel calib(comms, subset);
            ASKAPLOG_INFO_STR(logger, "ASKAP synthesis calibrator " << ASKAP_PACKAGE_VERSION);

            if (comms.isMaster()) {
                ASKAPLOG_INFO_STR(logger, "parset file " << inputsPar.getValue());
                ASKAPLOG_INFO_STR(logger, parset);
            }

            const int nCycles = subset.getInt32("ncycles", 1);
            ASKAPCHECK(nCycles >= 0, " Number of calibration iterations should be a non-negative number, you have " <<
                       nCycles);

            for (int cycle = 0; cycle < nCycles; ++cycle) {
                ASKAPLOG_INFO_STR(logger, "*** Starting calibration iteration " << cycle + 1 << " ***");
                calib.broadcastModel();
                calib.receiveModel();
                calib.calcNE();
                calib.solveNE();
                ASKAPLOG_INFO_STR(logger,  "user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real());
            }

            ASKAPLOG_INFO_STR(logger,  "*** Finished calibration cycles ***");
            /*
            calib.broadcastModel();
            calib.receiveModel();
            calib.calcNE();
            calib.receiveNE();
            */
            calib.writeModel();
        }
        ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                              << " real:   " << timer.real());
        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        std::cerr << "Usage: " << argv[0] << " [-inputs parsetFile]" << std::endl;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    return 0;
}
