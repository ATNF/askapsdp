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
#include <askap_cpingest.h>

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

ASKAP_LOGGER(logger, ".TaskFactory");

using namespace askap;
using namespace askap::cp::ingest;

TaskFactory::TaskFactory(const LOFAR::ParameterSet& configParset)
    : itsConfigParset(configParset)
{
}

ITask::ShPtr TaskFactory::createTask(const LOFAR::ParameterSet& parset)
{
    // Extract task type & parameters
    const std::string type = parset.getString("type");
    LOFAR::ParameterSet params = parset.makeSubset("params.");

    // Merge the system configuration parset into the params
    params.adoptCollection(itsConfigParset, "config.");

    // Create the task
    ITask::ShPtr task;
    if (type == "CalcUVWTask") {
        task.reset(new CalcUVWTask(params));
    } else if (type == "CalTask") {
        task.reset(new CalTask(params));
    } else if (type == "ChannelAvgTask") {
        task.reset(new ChannelAvgTask(params));
    } else if (type == "MSSink") {
        task.reset(new MSSink(params));
    } else {
        ASKAPTHROW(AskapError, "Unknown task type specified");
    }
    return task;
}

boost::shared_ptr< MergedSource > TaskFactory::createSource(const LOFAR::ParameterSet& parset)
{
    // 1) Configure and create the metadata source
    const LOFAR::ParameterSet mdSubset = parset.makeSubset("MergedSource.metadata_source.");
    const std::string mdLocatorHost = mdSubset.getString("ice.locator_host");
    const std::string mdLocatorPort = mdSubset.getString("ice.locator_port");
    const std::string mdTopicManager = mdSubset.getString("icestorm.topicmanager");
    const std::string mdTopic = mdSubset.getString("icestorm.topic");
    const unsigned int mdBufSz = mdSubset.getUint32("buffer_size", 12);
    const std::string mdAdapterName = "IngestPipeline";
    IMetadataSource::ShPtr metadataSrc(new MetadataSource(mdLocatorHost,
                mdLocatorPort, mdTopicManager, mdTopic, mdAdapterName, mdBufSz));

    // 2) Configure and create the visibility source
    const LOFAR::ParameterSet visSubset = parset.makeSubset("MergedSource.vis_source.");
    const unsigned int visPort = visSubset.getUint32("port");
    const unsigned int defaultBufSz = 666 * 36 * 19 * 2;
    const unsigned int visBufSz = visSubset.getUint32("buffer_size", defaultBufSz);
    int rank, numTasks;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
    VisSource::ShPtr visSrc(new VisSource(visPort + rank, visBufSz));

    // 3) Create and configure the merged source
    boost::shared_ptr< MergedSource > source(new MergedSource(metadataSrc, visSrc, numTasks));
    return source;
}
