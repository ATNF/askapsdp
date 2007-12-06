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

#include <conrad/ConradError.h>

#include <parallel/DuchampParallel.h>

#include <sstream>

using namespace std;
using namespace conrad;
using namespace conrad::cp;
using namespace conrad::scimath;

namespace conrad
{
  namespace synthesis
  {

    DuchampParallel::DuchampParallel(int argc, const char** argv,
        const LOFAR::ACC::APS::ParameterSet& parset)
    : ConradParallel(argc, argv)
    {
 
      os() << "Construct me here" << std::endl;

    }

    // Find the lists (on the workers)
    void DuchampParallel::findLists()
    {
      if(isWorker()) {
        os() << "Finding lists" << std::endl;
      }
      else {
      }
      
    }
    
    // Condense the lists (on the master)
    void DuchampParallel::condenseLists() 
    {
      if(isMaster()) {
        os() << "Condensing lists" << std::endl;
      }
      else {
      }
    }
    
    // Find the statistics (on the workers)
    void DuchampParallel::findStatistics()
    {
      if(isWorker()) {
        os() << "Finding Statistics" << std::endl;
      }
      else {
      }
    }

  }
}
