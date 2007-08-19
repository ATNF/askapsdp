///
/// @file 
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#include <conrad/ConradError.h>

#include <parallel/SimParallel.h>

#include <APS/ParameterSet.h>

using std::cout;
using std::endl;

using namespace std;
using namespace conrad;
using namespace conrad::synthesis;

// Main function

int main(int argc, const char** argv)
{
	try
	{

		LOFAR::ACC::APS::ParameterSet parset("csimulator.in");
		LOFAR::ACC::APS::ParameterSet subset(parset.makeSubset("Csimulator."));

		SimParallel sim(argc, argv, subset);
		sim.os() << "CONRAD simulation program" << std::endl;
		
		sim.simulate();

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

