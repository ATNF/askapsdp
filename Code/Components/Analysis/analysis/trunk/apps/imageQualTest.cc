///
/// @file : Match output list eg. from cduchamp with known input list
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>

#include <Common/ParameterSet.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include <patternmatching/Matcher.h>
#include <patternmatching/GrothTriangles.h>

#include <parallelanalysis/DuchampParallel.h>

using namespace askap;
using namespace askap::analysis;
using namespace askap::analysis::matching;

ASKAP_LOGGER(logger, "imageQualTest.log");

// Move to Askap Util?
std::string getInputs(const std::string& key, const std::string& def, int argc,
                      const char** argv)
{
    if (argc > 2) {
        for (int arg = 0; arg < (argc - 1); arg++) {
            std::string argument = std::string(argv[arg]);

            if (argument == key) {
                return std::string(argv[arg+1]);
            }
        }
    }

    return def;
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
        std::string parsetFile(getInputs("-inputs", "imageQualTest.in", argc, argv));
        LOFAR::ParameterSet parset(parsetFile);
        LOFAR::ParameterSet subset(parset.makeSubset("imageQual."));
        DuchampParallel image(argc, argv, subset);
        ASKAPLOG_INFO_STR(logger,  "parset file " << parsetFile);
        image.getMetadata();
        ASKAPLOG_INFO_STR(logger, "Read image metadata");
        Matcher matcher(subset);
        matcher.setHeader(image.cube().header());
        matcher.readLists();
        bool doFixRef = subset.getBool("convolveReference", true);

        if (doFixRef) matcher.fixRefList(image.getBeamInfo());

        matcher.setTriangleLists();
        matcher.findMatches();
        matcher.findOffsets();
        matcher.addNewMatches();
        matcher.outputLists();
        ASKAPLOG_INFO_STR(logger, "Time for execution of imageQualTest = " << timer.real() << " sec");
    } catch (askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    exit(0);
}

