/// @file FlagTask.h
///
/// @copyright (c) 2014 CSIRO
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

#ifndef ASKAP_CP_INGEST_FLAGTASK_H
#define ASKAP_CP_INGEST_FLAGTASK_H

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {

/// @brief Flagging task for the central processor ingest pipeline.
///
/// This class encapsulates a task which runs as part of the central processor
/// ingest pipeline. This task is responsible for flagging visibilities.
///
/// This class implements the ITask interface which specified the process()
/// method. These "tasks" are treated polymorphically by the ingest pipeline.
/// Once data is sourced into the pipeline, the process() method is called
/// for each task (in a specific sequence), the VisChunk is read and/or modified
/// by each task.
///
/// The parameter set can contain the following three parameters. If neither
/// a cross-correlation or auto-correlation threshold is specified this task
/// does not mutate the VisChunk.
///
/// The cross-correlation threshold parameter specifies the upper limit of
/// allowed amplitude of cross-correlations. Cross-correlations with amplitude
/// greater than this will be flagged.
/// @verbatim
/// threshold.crosscorr = 10.0
/// @endverbatim
///
/// The auto-correlation threshold parameter specifies the upper limit of
/// allowed amplitude of auto-correlations. Auto-correlations with amplitude
/// greater than this will be flagged.
/// @verbatim
/// threshold.autocorr = 1e8
/// @endverbatim
///
/// The "zeroflagged", if true, will result in the visibilities which exceed
/// above defined thresholds being zero'd. The flag will also be set.
/// @verbatim
/// zeroflagged = true
/// @endverbatim
class FlagTask : public askap::cp::ingest::ITask {
    public:

        /// @brief Constructor
        /// @param[in] parset the configuration parameter set.
        /// @param[in] config configuration
        FlagTask(const LOFAR::ParameterSet& parset,
                        const Configuration& config);

        /// @brief destructor
        ~FlagTask();

        /// @brief Flag visibilities in the specified VisChunk.
        ///
        /// @param[in,out] chunk  the instance of VisChunk for which the
        ///                       flags will be applied.
        virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

    private:

        /// Upper limit of allowed amplitude of cross-correlations.
        /// Visibilities (cross-correlations) with amplitude greater than this
        /// will be flagged.
        float itsCrossCorrThreshold;

        /// True if a cross-correlation was set in the parset
        bool itsCrossCorrThresholdSet;

        /// Upper limit of allowed amplitude of auto-correlations.
        /// Visibilities (auto-correlations) with amplitude greater than this
        /// will be flagged.
        float itsAutoCorrThreshold;

        /// True if a auto-correlation was set in the parset
        bool itsAutoCorrThresholdSet;

        /// True if flagged visibilities should also be set to zero
        bool itsZeroFlagged;
};

} // ingest
} // cp
} // askap

#endif

