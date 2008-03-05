/// @file
/// 
/// @brief Perform calibration and write result in the parset file
/// @details This application performs calibration of a measurement set
/// and writes the solution to an external parset file 
/// 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>


#include <askap_synthesis.h>
#include <askap/AskapLogging.h>

#include <askap/AskapError.h>


#include <casa/OS/Timer.h>

#include <CommandLineParser.h>
#include <APS/ParameterSet.h>
#include <parallel/CalibratorParallel.h>


ASKAP_LOGGER(logger, ".ccalibrator");

using namespace std;
using namespace askap;
using namespace askap::synthesis;
using namespace cmdlineparser;

// Main function

int main(int argc, const char** argv)
{
  try {
      
    casa::Timer timer;
      
      
    timer.mark();
      
    {
      cmdlineparser::Parser parser; // a command line parser
      // command line parameter
      cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", 
                                                           "ccalibrator.in");
      // this parameter is optional                                                   
      parser.add(inputsPar, cmdlineparser::Parser::return_default);                                                           
        
      // I hope const_cast is temporary here
      parser.process(argc,const_cast<char**>(argv));
        
        
	  LOFAR::ACC::APS::ParameterSet parset(inputsPar);
	  LOFAR::ACC::APS::ParameterSet subset(parset.makeSubset("Ccalibrator."));
	
	  CalibratorParallel calib(argc, argv, subset);
	  
	  ASKAPLOG_INIT("ccalibrator.log_cfg");
	 
	  ASKAPLOG_INFO_STR(logger, "ASKAP synthesis calibrator " << ASKAP_PACKAGE_VERSION);
      
      if (calib.isMaster()) {
          ASKAPLOG_INFO_STR(logger, "parset file "<<inputsPar.getValue());
          ASKAPLOG_INFO_STR(logger, parset);
      }
      
      int nCycles = subset.getInt32("ncycles", 1);
      ASKAPCHECK(nCycles>=0, " Number of calibration iterations should be a non-negative number, you have "<<
                  nCycles);
                  
      for (int cycle = 0; cycle<nCycles; ++cycle) {
           ASKAPLOG_INFO_STR(logger, "*** Starting calibration iteration "<<cycle+1<<" ***");
           calib.broadcastModel();
           calib.receiveModel();
           calib.calcNE();
           calib.solveNE();
           ASKAPLOG_INFO_STR(logger,  "user:   " << timer.user () << " system: " << timer.system ()
                                   <<" real:   " << timer.real () );
      }
      ASKAPLOG_INFO_STR(logger,  "*** Finished calibration cycles ***" );
      /*
      calib.broadcastModel();
      calib.receiveModel();
      calib.calcNE();
      calib.receiveNE();
      */
      calib.writeModel();  
    }
    ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user () << " system: " << timer.system ()
			 <<" real:   " << timer.real () );
    ///==============================================================================
  }
  catch (const cmdlineparser::XParser &ex) {
      ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
      std::cerr<<"Usage: "<<argv[0]<<" [-inputs parsetFile]"<<std::endl;     
  }  
  catch (const askap::AskapError& x)
    {
      ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
      std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    }
  catch (const std::exception& x)
    {
      ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
      std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    }
  exit(0);
}
	    
