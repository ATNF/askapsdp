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
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>

#include <CommandLineParser.h>

#include <parallel/ImagerParallel.h>

#include <measurementequation/MEParsetInterface.h>

#include <fitting/Params.h>

#include <stdexcept>
#include <iostream>

#include <casa/OS/Timer.h>

ASKAP_LOGGER(logger, ".cimager");


using namespace askap;
using namespace askap::synthesis;
using namespace askap::scimath;
using namespace LOFAR::ACC::APS;

// Main function
int main(int argc, const char** argv) {
	try {

	  // Ensure that CASA log messages are captured
	  casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
	  casa::LogSink::globalSink (globalSink);

		casa::Timer timer;

		timer.mark();

		// Put everything in scope to ensure that all destructors are called 
		// before the final message
		{

			cmdlineparser::Parser parser; // a command line parser
			// command line parameter
			cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs",
					"cimager.in");
			// this parameter is optional                                                   
			parser.add(inputsPar, cmdlineparser::Parser::return_default);

			// I hope const_cast is temporary here
			parser.process(argc, const_cast<char**> (argv));

			const std::string parsetFile = inputsPar;

			ParameterSet parset(parsetFile);
			ParameterSet subset(parset.makeSubset("Cimager."));

			// We cannot issue log messages until MPI is initialized!
			ImagerParallel imager(argc, argv, subset);

			ASKAPLOG_INFO_STR(logger, "ASKAP synthesis imager " << ASKAP_PACKAGE_VERSION);

			if (imager.isMaster()) {
				ASKAPLOG_INFO_STR(logger, "parset file " << parsetFile );
				ASKAPLOG_INFO_STR(logger, parset);
			}

			int nCycles = subset.getInt32("ncycles", 0);
			if (nCycles == 0) {
				/// No cycling - just make a dirty image
				imager.broadcastModel();
				imager.receiveModel();
				imager.calcNE();
				imager.solveNE();
			} else {
				/// Perform multiple major cycles
				for (int cycle = 0; cycle < nCycles; cycle++) {
					ASKAPLOG_INFO_STR(logger, "*** Starting major cycle " << cycle << " ***" );
					imager.broadcastModel();
					imager.receiveModel();
					imager.calcNE();
					imager.solveNE();

					ASKAPLOG_INFO_STR(logger, "user:   " << timer.user () << " system: " << timer.system ()
							<<" real:   " << timer.real () );
				}
				ASKAPLOG_INFO_STR(logger, "*** Finished major cycles ***" );
				imager.broadcastModel();
				imager.receiveModel();
				imager.calcNE();
				imager.receiveNE();
			}

			/// This is the final step - restore the image and write it out
			imager.writeModel();
		}
		ASKAPLOG_INFO_STR(logger, "Total times - user:   " << timer.user () << " system: " << timer.system ()
				<<" real:   " << timer.real () );

		///==============================================================================
	} catch (const cmdlineparser::XParser &ex) {
		ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
		std::cerr << "Usage: " << argv[0] << " [-inputs parsetFile]"
				<< std::endl;
	}

	catch (const askap::AskapError& x) {
		ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
		std::cerr << "Askap error in " << argv[0] << ": " << x.what()
				<< std::endl;
		exit(1);
	} catch (const std::exception& x) {
		ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
		std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what()
				<< std::endl;
		exit(1);
	}
	exit(0);
}

