///
/// @file : Match output list eg. from cduchamp with known input list
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
#include <askap_evaluation.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <APS/ParameterSet.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include <patternmatching/Matcher.h>
#include <patternmatching/GrothTriangles.h>

using namespace askap;
using namespace askap::evaluation;
using namespace askap::evaluation::matching;
using namespace LOFAR::ACC::APS;

std::vector<Point> getPixList(std::ifstream &fin);
std::vector<Point> getBasePixList(std::ifstream &fin);
std::vector<Point> getResultsPixList(std::ifstream &fin);

ASKAP_LOGGER(logger, "imageQualTest.log");

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
  ASKAPLOG_INIT("cduchamp.log_cfg");

  try
  {

    //    casa::Timer timer;

    //    timer.mark();

    std::string parsetFile(getInputs("-inputs", "imageQualTest.in", argc, argv));

    ParameterSet parset(parsetFile);
    ParameterSet subset(parset.makeSubset("imageQual."));

    Matcher matcher(argc,argv,subset);

    matcher.setTriangleLists();

    matcher.findMatches();

    matcher.findOffsets();

    matcher.outputLists();
   
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

