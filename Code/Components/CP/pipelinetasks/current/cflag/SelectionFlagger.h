/// @file SelectionFlagger.h
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

#ifndef ASKAP_CP_PIPELINETASKS_SELECTIONFLAGGER_H
#define ASKAP_CP_PIPELINETASKS_SELECTIONFLAGGER_H

// System includes
#include <vector>
#include <set>
#include <stdint.h>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "ms/MeasurementSets/MSSelection.h"

// Local package includes
#include "cflag/IFlagger.h"
#include "cflag/FlaggingStats.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief A selection based flagging flagger. This allows flagging based on:
/// - Baseline (i.e. an antenna or a pair of antennas)
/// - Field index number
/// - Time range
/// - Scan index number
/// - Feed/beam index number
/// - UVRange
/// - Autocorrelations only
/// - Spectral (e.g. channel index number or frequency)
class SelectionFlagger : public IFlagger {
    public:

        /// @brief Constructor
        SelectionFlagger(const LOFAR::ParameterSet& parset,
                          const casa::MeasurementSet& ms);

        /// @see IFlagger::processRow()
        virtual void processRow(casa::MSColumns& msc, const casa::uInt row,
                                const bool dryRun);

        /// @see IFlagger::stats()
        virtual FlaggingStats stats(void) const;

    private:
        enum SelectionCriteria {
            BASELINE,
            FIELD,
            TIMERANGE,
            SCAN,
            FEED,
            AUTOCORR
        };

        bool checkBaseline(casa::MSColumns& msc, const casa::uInt row);
        bool checkField(casa::MSColumns& msc, const casa::uInt row);
        bool checkTimerange(casa::MSColumns& msc, const casa::uInt row);
        bool checkScan(casa::MSColumns& msc, const casa::uInt row);
        bool checkFeed(casa::MSColumns& msc, const casa::uInt row);
        bool checkAutocorr(casa::MSColumns& msc, const casa::uInt row);

        bool dispatch(const std::vector<SelectionCriteria>& v,
                      casa::MSColumns& msc, const casa::uInt row);

        void checkDetailed(casa::MSColumns& msc, const casa::uInt row,
                           const bool dryRun);

        // Sets the row flag to true, and also sets the flag true for each visibility
        void flagRow(casa::MSColumns& msc, const casa::uInt row, const bool dryRun);

        // Flagging statistics
        FlaggingStats itsStats;

        // The bulk of the parsing of selection criteria is delegated to this class
        casa::MSSelection itsSelection;

        // True of auto-correlations should be flagged.
        bool itsFlagAutoCorr;

        // Set to true if per channel or per polarisation product flagging
        // criteria is specified
        bool itsDetailedCriteriaExists;

        // A list indicating which of the row based selection criteria has
        // been specified. The criteria which are more granular than whole row
        // are indicated via the itsDetailedCriteriaExists attribute.
        std::vector<SelectionCriteria> itsRowCriteria;

        // A set containing the feeds that should be flagged.
        std::set<uint32_t> itsFeedsFlagged;
};

}
}
}

#endif
