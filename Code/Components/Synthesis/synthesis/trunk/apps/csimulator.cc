///
/// @file 
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>

#include <conrad/ConradError.h>

#include <parallel/SimParallel.h>

#include <APS/ParameterSet.h>

#include <casa/OS/Timer.h>

#include <CommandLineParser.h>

CONRAD_LOGGER(logger, ".csimulator");

using namespace std;
using namespace conrad;
using namespace conrad::synthesis;

// Main function

int main(int argc, const char** argv)
{
  try
    {
      
      casa::Timer timer;
      
      
      timer.mark();
      
      {
        cmdlineparser::Parser parser; // a command line parser
        // command line parameter
        cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", 
                                                           "csimulator.in");
        // this parameter is optional                                                   
        parser.add(inputsPar, cmdlineparser::Parser::return_default);                                                           
        
        // I hope const_cast is temporary here
        parser.process(argc,const_cast<char**>(argv));
        
        // we could have used inputsPar directly in the code below
	    const std::string parsetFile = inputsPar;
	    
	LOFAR::ACC::APS::ParameterSet parset(parsetFile);
	LOFAR::ACC::APS::ParameterSet subset(parset.makeSubset("Csimulator."));
	
	SimParallel sim(argc, argv, subset);
	CONRADLOG_INIT("csimulator.log_cfg");

	CONRADLOG_INFO_STR(logger, "CONRAD synthesis simulator " << ASKAP_PACKAGE_VERSION);
	
	if(sim.isMaster()) {
	  CONRADLOG_INFO_STR(logger,  "parset file " << parsetFile );
	  CONRADLOG_INFO_STR(logger,  parset);
	}
	
	sim.init();
	
	sim.simulate();
      }
      CONRADLOG_INFO_STR(logger,  "Total times - user:   " << timer.user () << " system: " << timer.system ()
			 <<" real:   " << timer.real () );
      
      ///==============================================================================
    }
  catch (const cmdlineparser::XParser &ex) {
      CONRADLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
      std::cerr<<"Usage: "<<argv[0]<<" [-inputs parsetFile]"<<std::endl;     
  }  
  catch (const conrad::ConradError& x)
    {
      CONRADLOG_FATAL_STR(logger, "Conrad error in " << argv[0] << ": " << x.what());
      std::cerr << "Conrad error in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    }
  catch (const std::exception& x)
    {
      CONRADLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
      std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    }
  exit(0);
}

