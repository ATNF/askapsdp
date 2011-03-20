/// @file UVPublishTask.h
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

#ifndef ASKAP_CP_INGEST_UVPUBLISHTASK_H
#define ASKAP_CP_INGEST_UVPUBLISHTASK_H

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"
#include "uvchannel/UVChannelPublisher.h"

// Local package includes
#include "ingestpipeline/ITask.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief  Publish the data stream to the uv-channel
class UVPublishTask : public askap::cp::ingest::ITask {
    public:

        /// @brief Constructor.
        /// @param[in] parset the configuration parameter set.
        UVPublishTask(const LOFAR::ParameterSet& parset);

        /// @brief Destructor.
        virtual ~UVPublishTask();

        /// @brief Process.
        ///
        /// @param[in,out] chunk  the instance of VisChunk which will be
        /// distributed via the uv-channel.
        virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

    private:

        // Publisher object to publish to the uv-channel
        boost::shared_ptr<askap::cp::channels::UVChannelPublisher> itsPublisher;

        // MPI rank
        int itsRank;
};

}
}
}

#endif
