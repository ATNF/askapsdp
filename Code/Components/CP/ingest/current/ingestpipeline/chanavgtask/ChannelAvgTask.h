/// @file ChannelAvgTask.h
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

#ifndef ASKAP_CP_INGEST_CHANNELAVGTASK_H
#define ASKAP_CP_INGEST_CHANNELAVGTASK_H

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "casa/aips.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/ITask.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief Channel averaging task for the central processor ingest pipeline.
///
/// This task requires a configuration entry in the parset passed to the
/// constructor. This configuration entry specifies how many channels to be
/// averaged to one. For example:
/// @verbatim
///     chanavg.averaging                = 54
/// @endverbatim
/// The above results in 54 channels being averaged to one. Note the number of
/// channels in the VisChunk must be a multple of this number.
class ChannelAvgTask : public askap::cp::ingest::ITask {
    public:
        /// @brief Constructor.
        /// @param[in] parset   the parameter set used to configure this task.
        ChannelAvgTask(const LOFAR::ParameterSet& parset);

        /// @brief Destructor.
        virtual ~ChannelAvgTask();

        /// @brief Averages channels in the VisChunk.
        ///
        /// @param[in,out] chunk the instance of VisChunk for which channels will
        ///             be averaged. This method manipulates the VisChunk
        ///             instance which is passed in, hence the value of the
        ///             pointer will be unchanged.
        virtual void process(VisChunk::ShPtr chunk);

    private:

        // Parameter set
        const LOFAR::ParameterSet itsParset;

        // Number of channels to average to one
        casa::uInt itsAveraging;
};

}
}
}

#endif
