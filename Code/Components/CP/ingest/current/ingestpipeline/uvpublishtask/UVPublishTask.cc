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

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"
#include "uvchannel/UVChannelPublisher.h"

// Local package includes
#include "ingestutils/ParsetConfiguration.h"

ASKAP_LOGGER(logger, ".UVPublishTask");

using namespace std;
using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;
using namespace askap::cp::channels;

UVPublishTask::UVPublishTask(const LOFAR::ParameterSet& parset)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");

    const LOFAR::ParameterSet uvSubset = parset.makeSubset("config.");

    const string channelName = parset.getString("channel_name");
    itsPublisher.reset(new UVChannelPublisher(uvSubset, channelName));
}

UVPublishTask::~UVPublishTask()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
    itsPublisher.reset();
}

void UVPublishTask::process(VisChunk::ShPtr chunk)
{
    ASKAPLOG_DEBUG_STR(logger, "process()");

    // TODO: Need to break up the chunk into single channels then publish.
    // also need to know the absolute channel number

    // Publish an empty vischunk for now
    VisChunk data(chunk->nRow(), 1, chunk->nPol());
    for (unsigned int i = 1; i <= chunk->nChannel(); ++i) {
        itsPublisher->publish(data, i);
    }
}
