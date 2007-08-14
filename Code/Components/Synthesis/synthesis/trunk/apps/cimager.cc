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
#include <conrad/ConradError.h>

#include <parallel/ImagerParallel.h>

#include <measurementequation/ParsetInterface.h>

#include <fitting/Params.h>

#include <stdexcept>
#include <iostream>

#include <casa/OS/Timer.h>

using std::cout;
using std::endl;

using namespace conrad;
using namespace conrad::synthesis;
using namespace conrad::scimath;
using namespace LOFAR::ACC::APS;

// Main function

int main(int argc, const char** argv)
{
	ParameterSet parset("cimager.in");

	ImagerParallel imager(argc, argv, parset);

	try
	{
		casa::Timer timer;
		timer.mark();
		
		imager.initialize();

		int nCycles=parset.getInt32("Cimager.ncycles", 0);
		if(nCycles==0)
		{
			/// No cycling - just make a dirty image
			imager.calcNE();
			imager.solveNE();
		}
		else
		{
			/// Perform multiple major cycles
			for (int cycle=0;cycle<nCycles;cycle++)
			{
				imager.os() << "*** Starting major cycle " << cycle << " ***" << std::endl;
				imager.calcNE();
				imager.solveNE();
				imager.os() << "user:   " << timer.user () << " system: " << timer.system ()
				<<" real:   " << timer.real () << std::endl;
			}
		}
		imager.finalize();
		/// This is the final step - restore the image and write it out
		imager.writeModel();

		///==============================================================================
		exit(0);
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
}

