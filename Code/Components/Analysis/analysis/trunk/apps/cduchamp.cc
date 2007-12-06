///
/// @file : Duchamp driver
///
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#include <conrad/ConradError.h>

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
  try
  {

    casa::Timer timer;

    timer.mark();

    std::string parsetFile(getInputs("-inputs", "cduchamp.in", argc, argv));

    ParameterSet parset(parsetFile);
    ParameterSet subset(parset.makeSubset("Cduchamp."));

    DuchampParallel duchamp(argc, argv, subset);
    duchamp.os() << "parset file " << parsetFile << std::endl;
    
    duchamp.findLists();
    duchamp.condenseLists();
    duchamp.findStatistics();

    ///==============================================================================
  }
  catch (conrad::ConradError& x)
  {
    std::cerr << "Conrad error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  catch (std::exception& x)
  {
    std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  exit(0);
}

