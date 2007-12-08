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

using namespace std;
using namespace conrad;
using namespace conrad::cp;

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
      }

    }

    // Find the lists (on the workers)
    void DuchampParallel::findLists()
    {
      if(isWorker()) {
        CONRADLOG_INFO_STR( "Finding lists from image " << itsImage);
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
        CONRADLOG_INFO_STR( "Condensing lists" );
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
        CONRADLOG_INFO_STR( "Finding Statistics" );
        // Send back the statistics to the master
      }
      else {
      }
    }

    // Print the statistics (on the master)
    void DuchampParallel::printStatistics()
    {
      if(isMaster()) {
        // Get the statistics from the workers
        CONRADLOG_INFO_STR( "Receiving Statistics" );
        // Print out
      }
      else {
      }
    }

  }
}
