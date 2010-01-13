/// @file SimPlayback.cc
///
/// @copyright (c) 2009 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "SimPlayback.h"

// Include package level header file
#include "askap_correlatorsim.h"

// System includes
#include <string>
#include <mpi.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"
#include "boost/scoped_ptr.hpp"


// Local package includes
#include "simplayback/ISimulator.h"
#include "simplayback/CorrelatorSimulator.h"
#include "simplayback/TosSimulator.h"

// Using
using namespace askap::cp;

ASKAP_LOGGER(logger, ".SimPlayback");

SimPlayback::SimPlayback(const LOFAR::ParameterSet& parset)
    : itsParset(parset)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &itsRank);
    MPI_Comm_size(MPI_COMM_WORLD, &itsNumProcs);
}

SimPlayback::~SimPlayback()
{
}

void SimPlayback::run(void)
{
    if (itsRank == 0) {
        // Check there are enough ranks
        const int nShelves = itsParset.getUint32("playback.corrsim.n_shelves");
        ASKAPCHECK(itsNumProcs == (nShelves+1),
                "Incorrect number of ranks for the requested configration");
    }

    boost::scoped_ptr<ISimulator> sim;
    if (itsRank == 0) {
        sim.reset(new TosSimulator(itsParset));
    } else {
        sim.reset(new CorrelatorSimulator(itsParset));
    }

    ASKAPLOG_INFO_STR(logger, "Starting... ");
    bool moreData = true;
    while (moreData) {
        MPI_Barrier(MPI_COMM_WORLD);
        moreData = sim->fillNext();
    }

    ASKAPLOG_INFO_STR(logger, "Completed");
    sim.reset();
}
