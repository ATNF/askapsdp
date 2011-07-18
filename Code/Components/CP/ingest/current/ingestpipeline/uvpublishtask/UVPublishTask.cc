/// @file UVPublishTask.cc
///
/// @copyright (c) 2010 CSIRO
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
#include "UVPublishTask.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <vector>
#include <mpi.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"
#include "uvchannel/UVChannelPublisher.h"

// Local package includes
#include "ingestutils/ParsetConfiguration.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

ASKAP_LOGGER(logger, ".UVPublishTask");

using namespace std;
using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;
using namespace askap::cp::channels;

UVPublishTask::UVPublishTask(const LOFAR::ParameterSet& parset,
        const Configuration& /*config*/) : itsRank(-1)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");

    const LOFAR::ParameterSet uvSubset = parset.makeSubset("config.");

    const string channelName = parset.getString("channel_name");
    itsPublisher.reset(new UVChannelPublisher(uvSubset, channelName));

    int rc = MPI_Comm_rank(MPI_COMM_WORLD, &itsRank);
    if (rc != MPI_SUCCESS) {
        ASKAPTHROW(AskapError, "Cannot determine MPI rank");
    }
}

UVPublishTask::~UVPublishTask()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
    itsPublisher.reset();
}

void UVPublishTask::process(VisChunk::ShPtr chunk)
{
    ASKAPLOG_DEBUG_STR(logger, "process()");

    // The starting channel number for this ingest process
    // TODO: This assumes all ingest processes handle the same number of channels
    // each. It would be good to make this more flexible.
    // Note: The loop (and the data in the containers) is zero based, publication
    // to the uv-channel is one based, so 1 is added here.
    const int channelBase = (itsRank * chunk->nChannel()) + 1;
    ASKAPLOG_DEBUG_STR(logger, "Channel Base is: " << channelBase);

    VisChunk single(chunk->nRow(), 1, chunk->nPol());
    
    // Copy over all data to the new single channel chunk, except for the
    // channel dependent data which will be handled in the loop
    single.time() = chunk->time();
    single.interval() = chunk->interval();
    single.antenna1() = chunk->antenna1();
    single.antenna2() = chunk->antenna2();
    single.beam1() = chunk->beam1();
    single.beam2() = chunk->beam2();
    single.beam1PA() = chunk->beam1PA();
    single.beam2PA() = chunk->beam2PA();
    single.pointingDir1() = chunk->pointingDir1();
    single.pointingDir2() = chunk->pointingDir2();
    single.uvw() = chunk->uvw();
    single.stokes() = chunk->stokes();
    single.directionFrame() = chunk->directionFrame();

    // Build and publish an VisChunk per channel
    for (unsigned int i = 0; i < chunk->nChannel(); ++i) {
        single.frequency()(0) = chunk->frequency()(1);

        // TODO: Can probably optimise this by copying an entire plane
        const casa::uInt nRow = chunk->nRow();
        const casa::uInt nPol = chunk->nPol();
        for (casa::uInt row = 0; row < nRow; ++row) {
            for (casa::uInt pol = 0; pol < nPol; ++pol) {
                single.visibility()(row, 0, pol) = chunk->visibility()(row, i, pol);
                single.flag()(row, 0, pol) = chunk->flag()(row, i, pol);
            }
        }

        itsPublisher->publish(single, channelBase + i);
    }
}
