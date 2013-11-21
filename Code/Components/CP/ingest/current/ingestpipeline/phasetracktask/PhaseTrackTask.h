/// @file PhaseTrackTask.h
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

#ifndef ASKAP_CP_INGEST_PHASETRACKTASK_H
#define ASKAP_CP_INGEST_PHASETRACKTASK_H

// std includes
#include <vector>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/calcuvwtask/CalcUVWTask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {


/// @brief helper method to obtain effective LO frequency
/// @details The effective LO frequency is deduced from the sky frequency as
/// ASKAP has a simple conversion chain (the effective LO and the sky frequency of
/// the first channel always have a fixed offset which is hard coded). 
/// It is handy to encapsulate the formula in one method as it is used by more
/// than one class.
/// @param[in] config configuration object
/// @param[in] scan scan number
/// @return Effective LO frequency in Hz
double getEffectiveLOFreq(const Configuration &config, const casa::uInt scan);


/// @brief task to apply phase tracking
///
/// @details In the early version the hardware correlator may not do phase and delay tracking.
/// The delay tracking is done via DRx by offseting memory read out. The corresponding phase
/// tracking term is applied by this class. The effective LO frequency is not available as
/// part of metadata, but we can deduce it from the sky frequency (see getEffectiveLOFreq)
///
/// There are many common steps between this task and CalcUVWTask, but it seems better
/// not to merge phase tracking with the UVW calculator because we wouldn't need it in the long term.
/// For simplicity, this class is derived from UVW calculator class.
class PhaseTrackTask : public askap::cp::ingest::CalcUVWTask {
    public:

        /// @brief Constructor.
        /// @param[in] parset the configuration parameter set.
        PhaseTrackTask(const LOFAR::ParameterSet& parset,
                       const Configuration& config);

        /// @brief Phase-rotate visibilities in the specified VisChunk.
        ///
        /// @param[in,out] chunk  the instance of VisChunk for which the
        ///                       phase factors will be applied.
        virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

    protected:
        /// @brief phase rotate one row of the chunk
        /// @details
        /// @param[in] chunk vis chunk to work with
        /// @param[in] row the row of the chunk to work with
        void phaseRotateRow(askap::cp::common::VisChunk::ShPtr chunk, const casa::uInt row) const;

    private:
        /// @brief configuration (need scan information)
        Configuration itsConfig;

        /// @brief do delay tracking
        /// @details This is to control experimental feature to track delays and phases entirely
        /// in the software. This is intended to take the saw-tooth out, but may not be adequate
        /// for some positions in the sky
        bool itsTrackDelay;

        /// @brief if true, delays are assumed to be compensated via DRX for the South Pole,
        /// local zenith otherwise.
        bool itsTrackedSouthPole;

        /// @brief fixed delay component in ns
        ///
        /// @details This attribute constrols whether fixed delays are added. The values (one
        /// delay per antenna) are simply added to the geometric delay if tracked or applied
        /// as they are. If antenna ID exceeds the size of the vector, the delay is assumed to
        /// be zero. Zero length means no application of the fixed delay.
        ///
        /// @note if itsTrackDelay is false and the length of this vector is zero, only phase
        /// rotation is applied
        std::vector<double> itsFixedDelays;
}; // PhaseTrackTask class

} // ingest
} // cp
} // askap

#endif // #ifndef ASKAP_CP_INGEST_PHASETRACKTASK_H

