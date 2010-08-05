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

ASKAP_LOGGER(logger, ".TaskFactory");

using namespace askap;
using namespace askap::cp;

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
