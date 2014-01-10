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
#include <makeModelSlice/MakeAllModelSlicesApp.h>

// Include package level header file
#include <askap_simulations.h>

// System includes
#include <cstdlib>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>
#include <askap/StatReporter.h>
#include <Common/ParameterSet.h>

// Local package includes
#include <makeModelSlice/SliceMaker.h>

// Using
using namespace std;
using namespace askap;
using namespace askap::simulations;

ASKAP_LOGGER(logger, ".makeallmodelslicesapp");

int MakeAllModelSlicesApp::run(int argc, char* argv[])
{

    // This class must have scope outside the main try/catch block
    askap::askapparallel::AskapParallel comms(argc, const_cast<const char**>(argv));

    try {
	StatReporter stats;
		
	LOFAR::ParameterSet parset;
	parset.adoptCollection(config());
	LOFAR::ParameterSet subset(parset.makeSubset("makeAllModelSlices."));
	std::cout << "Initial parset:\n" << parset << "Subset of parset:\n" << subset;
	// ASKAPLOG_DEBUG_STR(logger, "Initial parset:\n"<<parset);
	// ASKAPLOG_DEBUG_STR(logger, "Subset of parset:\n"<<subset);
	
	size_t nchan = subset.getInt("nchan",0);
	ASKAPCHECK(nchan>0,"Number of channels in model needs to be provided with parameter 'nchan', which must be >0");
	size_t sliceWidth = subset.getInt("slicewidth",1);
	std::string slicebase = subset.getString("slicename");


	for (int chan=comms.rank()*sliceWidth; chan < nchan; chan += comms.nProcs()*sliceWidth) {

	    std::stringstream slicename;
	    slicename << slicebase << chan/sliceWidth;
	    std::stringstream range;
	    range << "[" << chan << "," << chan + sliceWidth - 1 << "]";

	    subset.replace(LOFAR::KVpair("chanRange",range));
	    subset.replace(LOFAR::KVpair("slicename",slicename.str()));

	    SliceMaker maker(subset);
	    maker.initialise();  // verify chunk list and set up coordsys
	    maker.createSlice(); // create the output image
	    maker.writeChunks(); // write the slice of each individual chunk

	}

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

