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


#include "ingestpipeline/derippletask/DerippleTask.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"


ASKAP_LOGGER(logger, ".DerippleTask");

using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;


/// @brief Constructor
DerippleTask::DerippleTask(const LOFAR::ParameterSet&, const Configuration&) 
{
  ASKAPLOG_DEBUG_STR(logger, "Constructor");
}

/// @brief destructor
DerippleTask::~DerippleTask()
{
}

const float DerippleTask::theirCoeffs[54] = 
  {1.024302,0.977712,0.986143,1.011121,1.024228,1.015525,
   0.994723,0.978709,0.978287,0.993027,1.012773,1.023972,
   1.018978,1.001655,0.983760,0.976310,0.983413,1.000749,
   1.017909,1.024255,1.015910,0.998346,0.982164,0.976335,
   0.983825,1.000569,1.017311,1.024300,1.017311,1.000569,
   0.983825,0.976335,0.982164,0.998346,1.015910,1.024255,
   1.017909,1.000749,0.983413,0.976310,0.983760,1.001655,
   1.018978,1.023972,1.012773,0.993027,0.978287,0.978709,
   0.994723,1.015525,1.024228,1.011121,0.986143,0.977712};

/// @brief Scale visibilities in the specified VisChunk.
/// @details This method applies static scaling factors to correct for FFB ripple
///
/// @param[in,out] chunk  the instance of VisChunk for which the
///                       scaling factors will be applied.
void DerippleTask::process(askap::cp::common::VisChunk::ShPtr chunk)
{
   ASKAPDEBUGASSERT(chunk);
   //
   ASKAPCHECK(chunk->nChannel() % 54 == 0, "Support only chunks with complete coarse channels, e.g. 16416 fine channels, you have "<<chunk->nChannel());

   for (casa::uInt chan=0; chan<chunk->nChannel(); ++chan) {
        casa::Matrix<casa::Complex> slice = chunk->visibility().xzPlane(chan);
        slice *= static_cast<casa::Float>(theirCoeffs[chan % 54]);
   }
}

