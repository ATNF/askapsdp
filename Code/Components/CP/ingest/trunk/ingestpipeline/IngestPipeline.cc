/// @file IngestPipeline.cc
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
#include "IngestPipeline.h"

// Include package level header file
#include <askap_cpingest.h>

// Boost includes
#include <boost/scoped_ptr.hpp>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/datadef/VisChunk.h"
#include "ingestpipeline/sourcetask/MetadataSource.h"
#include "ingestpipeline/sourcetask/VisSource.h"
#include "ingestpipeline/sourcetask/MergedSource.h"
#include "ingestpipeline/calcuvwtask/CalcUVWTask.h"
#include "ingestpipeline/caltask/CalTask.h"
#include "ingestpipeline/chanavgtask/ChannelAvgTask.h"
#include "ingestpipeline/sinktask/MSSink.h"

ASKAP_LOGGER(logger, ".IngestPipeline");

using namespace askap;
using namespace askap::cp;

IngestPipeline::IngestPipeline(const LOFAR::ParameterSet& parset)
    : itsParset(parset), itsRunning(false)
{
}

IngestPipeline::~IngestPipeline()
{
}

void IngestPipeline::start(void)
{
    itsRunning = true;
    ingest();
}

void IngestPipeline::abort(void)
{
    itsRunning = false;
}

void IngestPipeline::ingest(void)
{
    // 1) Setup tasks
    createSource();
    createTask<CalcUVWTask>(itsParset);
    createTask<CalTask>(itsParset);
    createTask<ChannelAvgTask>(itsParset);
    createTask<MSSink>(itsParset);

    // 2) Process correlator integrations, one at a time
    while (itsRunning) {
        bool endOfStream = ingestOne();
        if (endOfStream) {
            itsRunning = false;
        }
    }

    // 3) Clean up tasks
    itsSource.reset();
}

bool IngestPipeline::ingestOne(void)
{
    ASKAPLOG_DEBUG_STR(logger, "Waiting for data");
    VisChunk::ShPtr chunk(itsSource->next());
    ASKAPLOG_DEBUG_STR(logger, "Received one VisChunk. Timestamp: "
            << chunk->time());

    // For each task call process on the VisChunk
    for (unsigned int i = 0; i < itsTasks.size(); ++i) {
        itsTasks[i]->process(chunk);
    }

    return false; // Not finished
}

void IngestPipeline::createSource(void)
{
    // 1) Configure and create the metadata source
    const LOFAR::ParameterSet mdSubset = itsParset.makeSubset("metadata_source.");
    const std::string mdLocatorHost = mdSubset.getString("ice.locator_host");
    const std::string mdLocatorPort = mdSubset.getString("ice.locator_port");
    const std::string mdTopicManager = mdSubset.getString("icestorm.topicmanager");
    const std::string mdTopic = mdSubset.getString("icestorm.topic");
    const unsigned int mdBufSz = mdSubset.getUint32("buffer_size", 12);
    const std::string mdAdapterName = "IngestPipeline";
    IMetadataSource::ShPtr metadataSrc(new MetadataSource(mdLocatorHost,
                mdLocatorPort, mdTopicManager, mdTopic, mdAdapterName, mdBufSz));

    // 2) Configure and create the visibility source
    const LOFAR::ParameterSet visSubset = itsParset.makeSubset("vis_source.");
    const unsigned int visPort = visSubset.getUint32("port");
    const unsigned int defaultBufSz = 666 * 36 * 19 * 2;
    const unsigned int visBufSz= visSubset.getUint32("buffer_size", defaultBufSz);
    VisSource::ShPtr visSrc(new VisSource(visPort, visBufSz));

    // 3) Create and configure the merged source
    itsSource.reset(new MergedSource(metadataSrc, visSrc));
}

template <class T>
void IngestPipeline::createTask(const LOFAR::ParameterSet& parset)
{
    ITask::ShPtr task(new T(parset));
    itsTasks.push_back(task);
}
