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

using namespace std;
using namespace conrad;
using namespace conrad::synthesis;
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

    CONRADLOG_INIT("");

		casa::Timer timer;
    

		timer.mark();

		std::string parsetFile(getInputs("-inputs", "csimulator.in", argc, argv));
		LOFAR::ACC::APS::ParameterSet parset(parsetFile);
		LOFAR::ACC::APS::ParameterSet subset(parset.makeSubset("Csimulator."));

		SimParallel sim(argc, argv, subset);
                CONRADLOG_INFO_STR( "CONRAD simulation program" );
                CONRADLOG_INFO_STR( "parset file " << parsetFile );

		sim.simulate();
                CONRADLOG_INFO_STR( "user:   " << timer.user () << " system: " << timer.system ()
                                    <<" real:   " << timer.real () );

		///==============================================================================
	}
  catch (conrad::ConradError& x)
  {
    CONRADLOG_FATAL_STR("Conrad error in " << argv[0] << ": " << x.what());
    std::cerr << "Conrad error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  catch (std::exception& x)
  {
    CONRADLOG_FATAL_STR("Unexpected exception in " << argv[0] << ": " << x.what());
    std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  exit(0);
}

