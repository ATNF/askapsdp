/// @file DerippleTask.h
///
/// @copyright (c) 2013 CSIRO
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

#ifndef ASKAP_CP_INGEST_DERIPPLETASK_H
#define ASKAP_CP_INGEST_DERIPPLETASK_H

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {

/// @brief task for correcting ripple caused by the FFB
/// @details This task is intended to be used in early commissioning experiments. Long-term future of it is
/// unclear. Ideally, this functionality should be with ioc or even in the hardware. But for now it is handy
/// to be able to correct the data in the hardware
class DerippleTask : public askap::cp::ingest::ITask {
    public:

        /// @brief Constructor
        /// @param[in] parset the configuration parameter set.
        /// @param[in] config configuration
        DerippleTask(const LOFAR::ParameterSet& parset,
                     const Configuration& config);

        /// @brief destructor
        ~DerippleTask();

        /// @brief Scale visibilities in the specified VisChunk.
        /// @details This method applies static scaling factors to correct for FFB ripple
        ///
        /// @param[in,out] chunk  the instance of VisChunk for which the
        ///                       scaling factors will be applied.
        virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

    private:

        /// @brief correction coefficients
        static const float theirCoeffs[54];

}; // DerippleTask class

} // ingest
} // cp
} // askap

#endif // #ifndef ASKAP_CP_INGEST_DERIPPLETASK_H

