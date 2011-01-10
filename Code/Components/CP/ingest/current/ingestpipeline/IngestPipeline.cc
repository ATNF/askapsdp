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

// System includes
#include <string>
#include <vector>
#include <sstream>

// Boost includes
#include <boost/scoped_ptr.hpp>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/TaskFactory.h"
#include "ingestpipeline/sourcetask/MergedSource.h"

ASKAP_LOGGER(logger, ".IngestPipeline");

using namespace askap;
using namespace askap::cp::ingest;

IngestPipeline::IngestPipeline(const LOFAR::ParameterSet& parset)
    : itsParset(parset), itsRunning(false),
    itsIntegrationsCount(0)
{
    itsIntegrationsExpected = itsParset.getUint32("integrations_expected");
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
    // 1) Create a Task Factory
    const LOFAR::ParameterSet configParset = itsParset.makeSubset("config.");
    TaskFactory factory(configParset);

    // 2) Setup source
    itsSource = factory.createSource(itsParset);

    // 3) Setup tasks
    const std::vector<std::string> tasklist = itsParset.getStringVector("tasklist");
    ASKAPLOG_DEBUG_STR(logger, "Setting up these tasks: " << tasklist);
    std::vector<std::string>::const_iterator it = tasklist.begin();
    while (it != tasklist.end()) {
        std::stringstream key;
        key << "task." << *it << ".";
        LOFAR::ParameterSet taskParset = itsParset.makeSubset(key.str());
        ITask::ShPtr task = factory.createTask(taskParset);
        itsTasks.push_back(task);
        ++it;
    }

    // 4) Process correlator integrations, one at a time
    while (itsRunning && (itsIntegrationsCount < itsIntegrationsExpected))  {
        bool endOfStream = ingestOne();
        itsIntegrationsCount++;
        if (endOfStream) {
            itsRunning = false;
        }
    }

    // 5) Clean up tasks
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
