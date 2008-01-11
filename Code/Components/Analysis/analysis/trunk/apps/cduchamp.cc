///
/// @file : Duchamp driver
///
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#include <conrad/ConradError.h>

#include <conrad_analysis.h>

#include <conrad/ConradLogging.h>

#include <parallelanalysis/DuchampParallel.h>

#include <APS/ParameterSet.h>

#include <stdexcept>
#include <iostream>

#include <casa/OS/Timer.h>

using std::cout;
using std::endl;

using namespace conrad;
using namespace conrad::analysis;
using namespace LOFAR::ACC::APS;

CONRAD_LOGGER(logger, "");

// Move to Conrad Util
std::string getInputs(const std::string& key, const std::string& def, int argc,
    const char** argv)
{
  if (argc>2)
  {
    for (int arg=0; arg<(argc-1); arg++)
    {
      std::string argument=std::string(argv[arg]);
      if (argument==key)
      {
        return std::string(argv[arg+1]);
      }
    }
  }
  return def;
}

// Main function
int main(int argc, const char** argv)
{
  CONRADLOG_INIT("cduchamp.log_cfg");

  try
  {

    casa::Timer timer;

    timer.mark();

    std::string parsetFile(getInputs("-inputs", "cduchamp.in", argc, argv));

    ParameterSet parset(parsetFile);
    ParameterSet subset(parset.makeSubset("Cduchamp."));

    DuchampParallel duchamp(argc, argv, subset);
    CONRADLOG_INFO_STR(logger,  "parset file " << parsetFile );
    
    duchamp.readData();
    //    duchamp.findLists();
    //    duchamp.condenseLists();
//     duchamp.findStatistics();
//     duchamp.printStatistics();
    duchamp.findMeans();
    duchamp.combineMeans();
    duchamp.broadcastMean();
    duchamp.findRMSs();
    duchamp.combineRMSs();

    ///==============================================================================
  }
  catch (conrad::ConradError& x)
  {
    CONRADLOG_FATAL_STR(logger, "Conrad error in " << argv[0] << ": " << x.what());
    std::cerr << "Conrad error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  catch (std::exception& x)
  {
    CONRADLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
    std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  exit(0);
}

