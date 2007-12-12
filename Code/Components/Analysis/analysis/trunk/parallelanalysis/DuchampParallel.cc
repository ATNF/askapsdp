/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
/// 

#include <casa/OS/Timer.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/Path.h>

#include <conrad_analysis.h>

#include <conrad/ConradLogging.h>

#include <conrad/ConradError.h>

#include <parallelanalysis/DuchampParallel.h>

#include <sstream>

#include <duchamp/duchamp.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Statistics.hh>

CONRAD_LOGGER(logger, ".parallelanalysis");

using namespace std;
using namespace conrad;
using namespace conrad::cp;

using namespace duchamp;

namespace conrad
{
  namespace analysis
  {

    DuchampParallel::DuchampParallel(int argc, const char** argv,
        const LOFAR::ACC::APS::ParameterSet& parset)
    : ConradParallel(argc, argv)
    {
      if(isWorker()) {
        itsImage = substitute(parset.getString("image"));

	itsCube.pars().setImageFile(itsImage);
	itsCube.pars().setVerbosity(false);
	CONRADLOG_INFO_STR(logger, "Defined the cube.");
	//	CONRADLOG_INFO_STR(logger, "Defined the cube. Its Param set is :" << itsCube.pars());

// 	ParameterSet subset(parset.makeSubset("param."));
// 	std::string param; // use this for temporary storage of parameters.
// 	param = subsitute(parset.getString("");
	

      }

    }

    // Read in the data from the image file (on the workers)
    void DuchampParallel::readData()
    {
      if(isWorker()) {

	if(itsCube.getCube()==duchamp::FAILURE){
	  CONRADLOG_ERROR_STR(logger, "Could not read in data from image " << itsImage);
	}
	else {
	  CONRADLOG_INFO_STR(logger,  "Read data from image " << itsImage);
	}

      }
      else {
      }

    }
      


    // Find the lists (on the workers)
    void DuchampParallel::findLists()
    {
      if(isWorker()) {
        CONRADLOG_INFO_STR(logger,  "Finding lists from image " << itsImage);
        // Send the lists to the master here
      }
      else {
      }
      
    }
    
    // Condense the lists (on the master)
    void DuchampParallel::condenseLists() 
    {
      if(isMaster()) {
        // Get the lists from the workers here
        // ....
        // Now process the lists
        CONRADLOG_INFO_STR(logger,  "Condensing lists" );
        // Send the region specifications to the workers
      }
      else {
      }
    }
    
    // Find the statistics (on the workers)
    void DuchampParallel::findStatistics()
    {
      if(isWorker()) {
        // Get the region specifications from the master
        CONRADLOG_INFO_STR(logger,  "Finding Statistics" );
        // Send back the statistics to the master
	itsCube.setCubeStats();
	CONRADLOG_INFO_STR(logger, "Mean = " << itsCube.stats().getMean() << ", RMS = " << itsCube.stats().getStddev() );
	CONRADLOG_INFO_STR(logger, "Median = " << itsCube.stats().getMedian() << ", MADFM = " << itsCube.stats().getMadfm() );
	CONRADLOG_INFO_STR(logger, "Noise level = " << itsCube.stats().getMiddle() << ", Noise spread = " << itsCube.stats().getSpread() );
      }
      else {
      }
    }

    // Print the statistics (on the master)
    void DuchampParallel::printStatistics()
    {
      if(isMaster()) {
        // Get the statistics from the workers
        CONRADLOG_INFO_STR(logger,  "Receiving Statistics" );
        // Print out
      }
      else {
      }
    }

  }
}
