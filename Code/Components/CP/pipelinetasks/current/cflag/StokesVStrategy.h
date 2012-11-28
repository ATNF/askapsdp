/// @file StokesVStrategy.h
///
/// @copyright (c) 2012 CSIRO
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

#ifndef ASKAP_CP_PIPELINETASKS_STOKESVSTRATEGY_H
#define ASKAP_CP_PIPELINETASKS_STOKESVSTRATEGY_H

// System includes
#include <vector>
#include <map>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "ms/MeasurementSets/MSPolColumns.h"
#include "ms/MeasurementSets/StokesConverter.h"

// Local package includes
#include "cflag/IFlagStrategy.h"
#include "cflag/FlaggingStats.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief Performs flagging based on Stokes-V thresholding. For each row
/// the mean and standard deviation for all Stokes-V correlations (i.e. all
/// channels within a given row). Then, where the stokes-V correlation exceeds
/// the average plus (stddev * threshold) all correlations for that channel in
/// that row will be flagged.
/// 
/// The one parameter that is read from the parset passed to the constructor is
/// "threshold". To flag at the five-sigma point specify a valud of "5.0".
class StokesVStrategy : public IFlagStrategy {
    public:

        /// @brief Constructor
        StokesVStrategy(const LOFAR::ParameterSet& parset,
                        const casa::MeasurementSet& ms);

        /// @see IFlagStrategy::processRow
        virtual void processRow(casa::MSColumns& msc, const casa::uInt row,
                                const bool dryRun);

        /// @see IFlagStrategy::stats
        virtual FlaggingStats stats(void) const;

    private:
        casa::StokesConverter& getStokesConverter(const casa::ROMSPolarizationColumns& polc,
                const casa::Int polId);

        // Flagging statistics
        FlaggingStats itsStats;

        // Flagging threshold (in standard deviations)
        float itsThreshold;

        // StokesConverter cache
        std::map<casa::Int, casa::StokesConverter> itsConverterCache;
};

}
}
}

#endif
