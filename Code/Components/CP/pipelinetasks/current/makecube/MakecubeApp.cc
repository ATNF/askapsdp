/// @file MakecubeApp.cc
///
/// @copyright (c) 2013 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>

// Include own header file first
#include <makecube/MakecubeApp.h>

// Include package level header file
#include <askap_pipelinetasks.h>

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>
#include <askap/StatReporter.h>

#include <Common/ParameterSet.h>

#include <makecube/CubeMaker.h>

// Local package includes

// Using
using namespace askap;
using namespace askap::cp::pipelinetasks;
using namespace casa;

ASKAP_LOGGER(logger, ".MakecubeApp");

int MakecubeApp::run(int argc, char* argv[])
{
    try {

	StatReporter stats;
	
	LOFAR::ParameterSet parset;
	parset.adoptCollection(config());
	LOFAR::ParameterSet subset(parset.makeSubset("Makecube."));
	
	CubeMaker cube(subset);
	cube.initialise();
	cube.createCube();
	cube.setImageInfo();
	cube.writeSlices();
	    
	stats.logSummary();

    } catch (const askap::AskapError& x) {
	ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
	std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
	exit(1);
    } catch (const std::exception& x) {
	ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
	std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
	exit(1);
    }


    return 0;
}
