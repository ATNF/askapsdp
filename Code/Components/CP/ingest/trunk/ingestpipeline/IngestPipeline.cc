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
#include "ingestpipeline/datadef/VisChunk.h"
#include "ingestpipeline/sourcetask/MetadataSource.h"
#include "ingestpipeline/sourcetask/VisSource.h"

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
    IMetadataSource::ShPtr metadataSrc(new MetadataSource("localhost", "4061", "TopicManager", "tosmetadata", "IngestPipeline", 30));
    VisSource::ShPtr visSrc(new VisSource(3000, 666 * 36 * 304 * 2));
    itsSource.reset(new MergedSource(metadataSrc, visSrc));

    // 2) Process correlator integrations, one at a time
    while (itsRunning) {
        ingestOne();
    }

    // 3) Clean up tasks
    itsSource.reset();
}

void IngestPipeline::ingestOne(void)
{
    VisChunk::ShPtr chunk(itsSource->next());
}
