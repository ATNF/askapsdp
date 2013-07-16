/// @file AmplitudeFlagger.h
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_PIPELINETASKS_AMPLITUDEFLAGGER_H
#define ASKAP_CP_PIPELINETASKS_AMPLITUDEFLAGGER_H

// System includes
#include <set>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "measures/Measures/Stokes.h"
#include "casa/Arrays/Vector.h"

// Local package includes
#include "cflag/IFlagger.h"
#include "cflag/FlaggingStats.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief Applies flagging based on amplitude thresholding.
class AmplitudeFlagger : public IFlagger {
    public:

        /// @brief Constructor
        /// @throw AskapError   If an upper or lower threshold is not specified
        ///                     in the parset.
        AmplitudeFlagger(const LOFAR::ParameterSet& parset,
                          const casa::MeasurementSet& ms);

        /// @see IFlagger::processRow()
        virtual void processRow(casa::MSColumns& msc, const casa::uInt row,
                                const bool dryRun);

        /// @see IFlagger::stats()
        virtual FlaggingStats stats(void) const;

    private:
        /// Returns a vector of stokes types for a given row in the main table
        /// of the measurement set. This will have the same dimension and
        /// ordering as the data/flag matrices.
        casa::Vector<casa::Int> getStokesType(casa::MSColumns& msc,
                                              const casa::uInt row);

        // Flagging statistics
        FlaggingStats itsStats;

        // True if an upper amplitude limit has been set, otherwise false
        bool itsHasHighLimit;

        // True if lower amplitude limit has been set, otherwise false
        bool itsHasLowLimit;

        // The upper amplitude limit
        casa::Float itsHighLimit;

        // The lower amplitude limit
        casa::Float itsLowLimit;

        // The set of correlation products for which these flagging rules should
        // be applied. An empty list means apply to all correlation products.
        std::set<casa::Stokes::StokesTypes> itsStokes;
};

}
}
}

#endif
