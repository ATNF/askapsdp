///
/// @file : Duchamp driver
///
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#include <askap/AskapError.h>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>

#include <parallelanalysis/DuchampParallel.h>

#include <duchamp/duchamp.hh>

#include <APS/ParameterSet.h>

#include <stdexcept>
#include <iostream>

#include <casa/OS/Timer.h>

using std::cout;
using std::endl;

using namespace askap;
using namespace askap::analysis;
using namespace LOFAR::ACC::APS;

ASKAP_LOGGER(logger, "cduchamp.log");

// Move to Askap Util
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

  try
  {

    casa::Timer timer;

    timer.mark();

    std::string parsetFile(getInputs("-inputs", "cduchamp.in", argc, argv));

    ParameterSet parset(parsetFile);
    ParameterSet subset(parset.makeSubset("Cduchamp."));

    DuchampParallel duchamp(argc, argv, subset);
    ASKAPLOG_INFO_STR(logger,  "parset file " << parsetFile );
    
    duchamp.readData();
    duchamp.gatherStats();
    duchamp.broadcastThreshold();
    duchamp.receiveThreshold();

    duchamp.findSources();
    duchamp.fitSources();
    duchamp.sendObjects();
    duchamp.receiveObjects();
    duchamp.cleanup();
    duchamp.printResults();

    //    duchamp.findLists();
    //    duchamp.condenseLists();
    //    duchamp.calcFluxes();
    //    duchamp.printResults();
    //    duchamp.fitSources();
    
    ASKAPLOG_INFO_STR(logger, "Time for execution of cduchamp = " << timer.real() << " sec");

    ///==============================================================================
  }
  catch (askap::AskapError& x)
  {
    ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
    std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  catch (std::exception& x)
  {
    ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
    std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  exit(0);
}

