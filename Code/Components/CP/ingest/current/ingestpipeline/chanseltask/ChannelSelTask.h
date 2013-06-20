/// @file ChannelSelTask.h
///
///
/// Selection of subset of channels
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

#ifndef ASKAP_CP_INGEST_CHANNELSELTASK_H
#define ASKAP_CP_INGEST_CHANNELSELTASK_H

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "casa/aips.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {

/// @brief Channel selection task for the central processor ingest pipeline.
/// @details This task is intended for the commissioning activities only. It keeps
/// a given subset of spectral channels and rejects everything else.
///
/// This task requires a configuration entry in the parset passed to the
/// constructor. This configuration entry specifies how many channels to be
/// passed and from what part of the band. For example:
/// @verbatim
///     chansel.start                = 2000
///     chansel.nchan                = 1000
/// @endverbatim
/// The above results in 1000 channels starting from channel 2000 to be selected. 
class ChannelSelTask : public askap::cp::ingest::ITask {
    public:
        /// @brief Constructor.
        /// @param[in] parset   the parameter set used to configure this task.
        /// @param[in] config   configuration
        ChannelSelTask(const LOFAR::ParameterSet& parset, const Configuration& config);

        /// @brief Destructor.
        virtual ~ChannelSelTask();

        /// @brief Selects channels in the VisChunk.
        ///
        /// @param[in,out] chunk the instance of VisChunk for which channels will
        ///             be selected. This method manipulates the VisChunk
        ///             instance which is passed in, hence the value of the
        ///             pointer will be unchanged.
        virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

    private:

        /// @brief Parameter set
        const LOFAR::ParameterSet itsParset;

        /// @brief First channel to select
        casa::uInt itsStart;

        /// @brief Number of channels to select
        casa::uInt itsNChan;
};

}
}
}

#endif
