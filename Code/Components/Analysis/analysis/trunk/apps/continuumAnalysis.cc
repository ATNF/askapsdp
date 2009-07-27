///
/// @file : Code to do all analysis and evaluation. Aimed at continuum data
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
#include <askap/AskapError.h>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>

#include <parallelanalysis/DuchampParallel.h>
#include <patternmatching/Matcher.h>
#include <patternmatching/GrothTriangles.h>

#include <duchamp/duchamp.hh>

#include <APS/ParameterSet.h>

#include <stdexcept>
#include <iostream>

#include <casa/OS/Timer.h>

using std::cout;
using std::endl;

using namespace askap;
using namespace askap::analysis;
using namespace askap::analysis::matching;
using namespace LOFAR::ACC::APS;

ASKAP_LOGGER(logger, "continuumAnalysis.log");

// Move to Askap Util
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
        casa::Timer timer;
        timer.mark();
        std::string parsetFile(getInputs("-inputs", "continuumAnalysis.in", argc, argv));
        ParameterSet parset(parsetFile);
        ParameterSet subsetD(parset.makeSubset("Cduchamp."));
        DuchampParallel image(argc, argv, subsetD);
        ASKAPLOG_INFO_STR(logger,  "parset file " << parsetFile);
        image.readData();
        image.setupLogfile(argc, argv);
        image.gatherStats();
        image.broadcastThreshold();
        image.receiveThreshold();
        image.findSources();
        image.fitSources();
        image.sendObjects();
        image.receiveObjects();
        image.cleanup();
        image.printResults();

        ParameterSet subsetE(parset.makeSubset("imageQual."));
        Matcher matcher(subsetE);
	matcher.setHeader(image.cube().header());
	matcher.readLists();
        matcher.fixRefList(image.getBeamInfo());
        matcher.setTriangleLists();
        matcher.findMatches();
        matcher.findOffsets();
        matcher.addNewMatches();
        matcher.outputLists();
        ASKAPLOG_INFO_STR(logger, "Time for execution of contAnalysis = " << timer.real() << " sec");
        ///==============================================================================
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

