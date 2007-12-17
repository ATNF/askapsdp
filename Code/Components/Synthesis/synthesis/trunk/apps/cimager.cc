///
/// @file : Synthesis imaging program
///
/// Performs synthesis imaging from a data source, using any of a number of
/// image solvers. Can run in serial or parallel (MPI) mode.
///
/// The data are accessed from the DataSource. This is and will probably remain
/// disk based. The images are kept purely in memory until the end.
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
#include <conrad/ConradError.h>


#include <parallel/ImagerParallel.h>

#include <measurementequation/MEParsetInterface.h>

#include <fitting/Params.h>

#include <stdexcept>
#include <iostream>

#include <casa/OS/Timer.h>

CONRAD_LOGGER(logger, ".cimager");

using namespace conrad;
using namespace conrad::synthesis;
using namespace conrad::scimath;
using namespace LOFAR::ACC::APS;

// Move to Conrad Util
std::string getInputs(const std::string& key, const std::string& def, int argc,
    const char** argv)
{
  if (argc>2)
  {
    for (int arg=0; arg<(argc-1); arg++)
    {
      std::string argument=string(argv[arg]);
      if (argument==key)
      {
        return string(argv[arg+1]);
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

    std::string parsetFile(getInputs("-inputs", "cimager.in", argc, argv));

    ParameterSet parset(parsetFile);
    ParameterSet subset(parset.makeSubset("Cimager."));

    ImagerParallel imager(argc, argv, subset);
    CONRADLOG_INIT("cimager.log_cfg");

    CONRADLOG_INFO_STR(logger,  "parset file " << parsetFile );

    int nCycles=subset.getInt32("ncycles", 0);
    if(nCycles==0)
    {
      /// No cycling - just make a dirty image
      imager.broadcastModel();
      imager.receiveModel();
      imager.calcNE();
      imager.solveNE();
    }
    else
    {
      /// Perform multiple major cycles
      for (int cycle=0;cycle<nCycles;cycle++)
      {
        CONRADLOG_INFO_STR(logger,  "*** Starting major cycle " << cycle << " ***" );
        imager.broadcastModel();
        imager.receiveModel();
        imager.calcNE();
        imager.solveNE();

        CONRADLOG_INFO_STR(logger,  "user:   " << timer.user () << " system: " << timer.system ()
                            <<" real:   " << timer.real () );
      }
      CONRADLOG_INFO_STR(logger,  "*** Finished major cycles ***" );
      imager.broadcastModel();
      imager.receiveModel();
      imager.calcNE();
      imager.receiveNE();
    }

    /// This is the final step - restore the image and write it out
    imager.writeModel();
    CONRADLOG_INFO_STR(logger,  "Total times - user:   " << timer.user () << " system: " << timer.system ()
		       <<" real:   " << timer.real () );

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

