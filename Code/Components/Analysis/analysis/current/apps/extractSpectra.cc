/// @file
///
/// Program to extract spectra for objects detected in a previous
/// cduchamp/selavy/duchamp run. 
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
// Package level header file
#include <askap_analysis.h>

// System includes
#include <stdexcept>
#include <iostream>

// ASKAPsoft includes
#include <askap/Application.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/StatReporter.h>
#include <askapparallel/AskapParallel.h>
#include <parallelanalysis/DuchampParallel.h>
#include <duchamp/duchamp.hh>
#include <Common/ParameterSet.h>

using std::cout;
using std::endl;
using namespace askap;
using namespace askap::analysis;

ASKAP_LOGGER(logger, "extractSpectra.log");

class SpectraApp : public askap::Application
{
public:
  virtual int run(int argc, char* argv[])
  {
    // This class must have scope outside the main try/catch block
    askap::askapparallel::AskapParallel comms(argc, const_cast<const char**>(argv));
    try {
      StatReporter stats;

      ASKAPLOG_INFO_STR(logger, "ASKAP source finder " << ASKAP_PACKAGE_VERSION);

      // Create a new parset with a nocase key compare policy, then
      // adopt the contents of the real parset
      LOFAR::ParameterSet parset(LOFAR::StringUtil::Compare::NOCASE);
      parset.adoptCollection(config());
      LOFAR::ParameterSet subset(parset.makeSubset("Spectra."));
      std::string previousLog = subset.getString("previousLogfile","duchamp-Logfile-Master.txt");
      if(!parset.isDefined("previousLogfile")) ASKAPLOG_WARN_STR(logger,"The parameter 'previousLogfile' is not defined - using default of " << previousLog);

      DuchampParallel duchamp(comms, subset);
      duchamp.cube().pars().setFlagUsePrevious(true);
      if(!comms.isParallel() || comms.isMaster())
	ASKAPLOG_INFO_STR(logger, "Parset file contents:\n" << config());
		
      duchamp.readData();
      duchamp.cube().pars().setLogFile(previousLog);
      ASKAPLOG_INFO_STR(logger, "Reading detections from previous log file " << previousLog);
      duchamp.cube().getExistingDetections();
      ASKAPLOG_INFO_STR(logger, "Cleaning up and parameterising detections");
      //      duchamp.cube().calcObjectParams();
      //      duchamp.cleanup();
      duchamp.finaliseDetection();
      ASKAPLOG_INFO_STR(logger, "Extracting requested spectra");
      duchamp.extractSpectra();
	    
      stats.logSummary();
      ///==============================================================================
    } catch (const askap::AskapError& x) {
      ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
      std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    } catch (const duchamp::DuchampError& x) {
      ASKAPLOG_FATAL_STR(logger, "Duchamp error in " << argv[0] << ": " << x.what());
      std::cerr << "Duchamp error in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    } catch (const std::exception& x) {
      ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
      std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    }

    return 0;
  }
};

int main(int argc, char *argv[])
{
  SpectraApp app;
  return app.main(argc, argv);
}
