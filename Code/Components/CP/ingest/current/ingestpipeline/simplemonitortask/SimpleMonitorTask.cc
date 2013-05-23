/// @file
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>


#include "ingestpipeline/simplemonitortask/SimpleMonitorTask.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"


// casa includes


ASKAP_LOGGER(logger, ".SimpleMonitorTask");

using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;


/// @breif Constructor
/// @param[in] parset the configuration parameter set.
SimpleMonitorTask::SimpleMonitorTask(const LOFAR::ParameterSet& parset,
             const Configuration&) : itsCurrentTime(-1.), itsStartTime(-1.)
{
  ASKAPLOG_DEBUG_STR(logger, "Constructor");
}

/// @brief destructor
SimpleMonitorTask::~SimpleMonitorTask()
{
}

/// @brief Extract required information from visibilities in the specified VisChunk.
/// @details There is no modification of the data, just internal buffers are updated.
///
/// @param[in,out] chunk  the instance of VisChunk for which the
///                       phase factors will be applied.
void SimpleMonitorTask::process(askap::cp::common::VisChunk::ShPtr chunk)
{
   ASKAPDEBUGASSERT(chunk);
   for (casa::uInt row=0; row<chunk->nRow(); ++row) {
   }
}

/// @details Process one row of data.
/// @param[in] vis vis matrix for the given row to work with
///                dimensions are channels and polarisations
/// @param[in] baseline baseline ID 
/// @param[in] beam beam ID
void SimpleMonitorTask::processRow(const casa::Matrix<casa::Complex> &vis, const casa::uInt baseline, const casa::uInt beam)
{
}

/// @brief Publish the buffer
void SimpleMonitorTask::publishBuffer() const
{
}

