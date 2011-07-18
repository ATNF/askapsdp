/// @file TaskFactory.cc
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
#include "TaskFactory.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <mpi.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/calcuvwtask/CalcUVWTask.h"
#include "ingestpipeline/caltask/CalTask.h"
#include "ingestpipeline/chanavgtask/ChannelAvgTask.h"
#include "ingestpipeline/mssink/MSSink.h"
#include "ingestpipeline/sourcetask/MetadataSource.h"
#include "ingestpipeline/sourcetask/VisSource.h"
#include "ingestpipeline/sourcetask/MergedSource.h"
#include "ingestpipeline/uvpublishtask/UVPublishTask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

ASKAP_LOGGER(logger, ".TaskFactory");

using namespace askap;
using namespace askap::cp::ingest;

TaskFactory::TaskFactory(const Configuration& config)
    : itsConfig(config)
{
}

ITask::ShPtr TaskFactory::createTask(const TaskDesc& taskDescription)
{
    // Extract task type & parameters
    const TaskDesc::Type type = taskDescription.type();
    const LOFAR::ParameterSet params = taskDescription.params();

    // Create the task
    ITask::ShPtr task;
    switch (type) {
        case TaskDesc::CalcUVWTask :
            task.reset(new CalcUVWTask(params, itsConfig));
            break;
        case  TaskDesc::CalTask :
            task.reset(new CalTask(params, itsConfig));
            break;
        case TaskDesc::ChannelAvgTask :
            task.reset(new ChannelAvgTask(params, itsConfig));
            break;
        case TaskDesc::MSSink :
            task.reset(new MSSink(params, itsConfig));
            break;
        case TaskDesc::UVPublishTask :
            task.reset(new UVPublishTask(params, itsConfig));
            break;
        default:
            ASKAPTHROW(AskapError, "Unknown task type specified");
            break;
    }

    return task;
}

boost::shared_ptr< MergedSource > TaskFactory::createSource(void)
{
    // Pre-conditions
    ASKAPCHECK(itsConfig.tasks().at(0).name().compare("MergedSource") == 0,
            "First defined task is not the Merged Source");

    // 1) Configure and create the metadata source
    const std::string mdLocatorHost = itsConfig.metadataTopic().registryHost();
    const std::string mdLocatorPort = itsConfig.metadataTopic().registryPort();
    const std::string mdTopicManager = itsConfig.metadataTopic().topicManager();
    const std::string mdTopic = itsConfig.metadataTopic().topic();
    const unsigned int mdBufSz = 12; // TODO: Make this a tunable
    const std::string mdAdapterName = "IngestPipeline"; // TODO: Eliminate this
    IMetadataSource::ShPtr metadataSrc(new MetadataSource(mdLocatorHost,
                mdLocatorPort, mdTopicManager, mdTopic, mdAdapterName, mdBufSz));

    // 2) Configure and create the visibility source
    const unsigned int visPort = itsConfig.tasks().at(0).params().getUint32("vis_source.port");
    const unsigned int defaultBufSz = 666 * 36 * 19 * 2;
    const unsigned int visBufSz = itsConfig.tasks().at(0).params().getUint32("buffer_size", defaultBufSz);
    int rank, numTasks;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
    VisSource::ShPtr visSrc(new VisSource(visPort + rank, visBufSz));

    // 3) Create and configure the merged source
    boost::shared_ptr< MergedSource > source(new MergedSource(metadataSrc, visSrc, numTasks));
    return source;
}
