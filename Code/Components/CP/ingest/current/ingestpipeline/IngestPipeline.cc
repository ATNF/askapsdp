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
#include "askap_cpingest.h"

// System includes
#include <string>
#include <vector>
#include <iterator>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/TaskFactory.h"
#include "ingestpipeline/sourcetask/MergedSource.h"
#include "ingestpipeline/sourcetask/NoMetadataSource.h"
#include "ingestpipeline/sourcetask/InterruptedException.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too
#include "monitoring/MonitoringSingleton.h"

ASKAP_LOGGER(logger, ".IngestPipeline");

using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;

IngestPipeline::IngestPipeline(const LOFAR::ParameterSet& parset,
                               int rank, int ntasks)
    : itsConfig(parset, rank, ntasks), itsRunning(false)
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
    // 1) Get task list from configuration
    const std::vector<TaskDesc>& tasks = itsConfig.tasks();

    // 2) Configure the Monitoring Singleton
    if (!itsConfig.monitoringArchiverService().registryHost().empty()) {
        MonitoringSingleton::init(itsConfig);
    }

    // 3) Create a Task Factory
    TaskFactory factory(itsConfig);

    // 4) Setup source
    if (tasks.empty()) {
        ASKAPTHROW(AskapError, "No pipeline tasks specified");
    }

    if (tasks[0].type() == TaskDesc::MergedSource) {
        itsSource = factory.createMergedSource();
    } else if (tasks[0].type() == TaskDesc::NoMetadataSource) {
        itsSource = factory.createNoMetadataSource();
    } else {
        ASKAPTHROW(AskapError, "First task should be a Source");
    }

    // 5) Setup tasks
    for (size_t i = 1; i < tasks.size(); ++i) {
        ITask::ShPtr task = factory.createTask(tasks[i]);
        itsTasks.push_back(task);
    }

    // 6) Process correlator integrations, one at a time
    while (itsRunning)  {
        try {
            bool endOfStream = ingestOne();
            itsRunning = !endOfStream;
        } catch (InterruptedException&) {
            break;
        }
    }

    // 7) Clean up
    itsSource.reset();
    if (MonitoringSingleton::instance()) {
        MonitoringSingleton::destroy();
    }
}

bool IngestPipeline::ingestOne(void)
{
    ASKAPLOG_DEBUG_STR(logger, "Waiting for data");
    VisChunk::ShPtr chunk(itsSource->next());
    if (chunk.get() == 0) {
        return true;
    }

    ASKAPLOG_INFO_STR(logger, "Received one VisChunk. Timestamp: "
            << chunk->time());

    // For each task call process on the VisChunk
    for (unsigned int i = 0; i < itsTasks.size(); ++i) {
        itsTasks[i]->process(chunk);
    }

    return false; // Not finished
}
