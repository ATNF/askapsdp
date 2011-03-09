///
/// @file : Duchamp driver
///
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

#include <askapparallel/AskapParallel.h>
#include <parallelanalysis/DuchampParallel.h>

#include <duchamp/duchamp.hh>

#include <Common/ParameterSet.h>

#include <stdexcept>
#include <iostream>

#include <casa/OS/Timer.h>

using std::cout;
using std::endl;

using namespace askap;
using namespace askap::analysis;

ASKAP_LOGGER(logger, "selavy.log");

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

void setSelavyNames(DuchampParallel &finder, askap::mwbase::AskapParallel &comms)
{
  finder.cube().pars().setOutFile("selavy-results.txt");
  finder.cube().pars().setHeaderFile("selavy-results.hdr");
  finder.cube().pars().setVOTFile("selavy-results.xml");
  finder.cube().pars().setKarmaFile("selavy-results.ann");
  if(comms.isParallel() && comms.isMaster())
    finder.cube().pars().setLogFile("selavy-Logfile-Master.txt");
  else
    finder.cube().pars().setLogFile(comms.substitute("selavy-Logfile-%w.txt"));

  finder.setSubimageAnnotationFile("selavy-SubimageLocations.ann");
  finder.setFitSummaryFile("selavy-fitResults.txt");
  finder.setFitAnnotationFile("selavy-fitResults.ann");
  finder.setFitBoxAnnotationFile("selavy-fitResults.boxes.ann");
  
}


// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::mwbase::AskapParallel comms(argc, argv);
    try {
        // Ensure that CASA log messages are captured
        casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
        casa::LogSink::globalSink(globalSink);

        casa::Timer timer;
        timer.mark();
        std::string parsetFile(getInputs("-inputs", "selavy.in", argc, argv));
        LOFAR::ParameterSet parset(parsetFile,LOFAR::StringUtil::Compare::NOCASE);
        LOFAR::ParameterSet subset(parset.makeSubset("Selavy."));
        if(!comms.isParallel() || comms.isMaster())
	  ASKAPLOG_INFO_STR(logger,  "parset file " << parsetFile);
        DuchampParallel finder(comms, subset);

	// change the default output names from duchamp-* to selavy-*
	setSelavyNames(finder,comms);

        finder.readData();
        finder.setupLogfile(argc, argv);
        finder.gatherStats();
        finder.broadcastThreshold();
        finder.receiveThreshold();
        finder.findSources();
        finder.fitSources();
        finder.sendObjects();
        finder.receiveObjects();
        finder.cleanup();
        finder.printResults();
        ASKAPLOG_INFO_STR(logger, "Time for execution of selavy = " << timer.real() << " sec");
        ///==============================================================================
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
