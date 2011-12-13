/// @file 
///
/// @brief converter from SWIN format to MS
/// @details This application is intended to convert DiFX output to a measurement set. 
/// It takes configuration paramters from the parset file, which
/// allows a flexible control over some parameters which we may need to change during the test 
/// (e.g. beam details, antenna locations, delay fudge factors). Same code is shared with the
/// real time software correlator, so parset parameters are the same.
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

// own includes
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <dataformats/SwinReader.h>
#include <swcorrelator/FillerMSSink.h>

// casa includes
#include <casa/OS/Timer.h>

// other 3rd party
#include <mwcommon/AskapParallel.h>
#include <Common/ParameterSet.h>
#include <CommandLineParser.h>

#include <vector>
#include <string>

ASKAP_LOGGER(logger, ".swcorrelator");

using namespace askap;
using namespace askap::swcorrelator;

// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::mwcommon::AskapParallel comms(argc, argv);
    
    try {
       casa::Timer timer;
       timer.mark();
       
       cmdlineparser::Parser parser; // a command line parser
       // command line parameter
       cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs",
                    "swin2ms.in");
       // this parameter is optional
       parser.add(inputsPar, cmdlineparser::Parser::return_default);

       parser.process(argc, argv);

       const LOFAR::ParameterSet parset(inputsPar);
       const LOFAR::ParameterSet subset(parset.makeSubset("swin2ms."));
       ASKAPCHECK(subset.isDefined("filename"), "Output file name should be defined in the parset!");
       FillerMSSink msSink(subset);
       const std::vector<std::string> names = subset.getStringVector("inputfiles");       
       SwinReader reader(msSink.nChan());
       ASKAPLOG_INFO_STR(logger,  "Conversion will assume "<<msSink.nChan()<<" spectral channels");
       CorrProducts cp(msSink.nChan(),0);
       for (std::vector<std::string>::const_iterator ci = names.begin(); ci!=names.end(); ++ci) {
            ASKAPLOG_INFO_STR(logger,  "Processing "<<*ci);
            casa::uInt counter = 0;
            for (reader.assign(*ci); reader.hasMore(); reader.next(), ++counter) {
            }
            ASKAPLOG_INFO_STR(logger,  "Read "<<counter<<" records");
       }
       
       ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                          << " real:   " << timer.real());
        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        ASKAPLOG_FATAL_STR(logger, "Usage: " << argv[0] << " [-inputs parsetFile]");
        return 1;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        return 1;
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        return 1;
    }

    return 0;
    
}

