/// @file ElevationFlagger.h
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

#ifndef ASKAP_CP_PIPELINETASKS_ELEVATIONFLAGGER_H
#define ASKAP_CP_PIPELINETASKS_ELEVATIONFLAGGER_H

// System includes
#include <vector>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "boost/shared_ptr.hpp"
#include "casa/aipstype.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "casa/Quanta/Quantum.h"

// Local package includes
#include "cflag/IFlagger.h"
#include "cflag/FlaggingStats.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief Applies flagging based on elevation. This flagger will flag any visibilities
/// where one or both of the antennas have an elevation either lower than the lower threshold
/// or higher than the upper threshold.
class ElevationFlagger : public IFlagger {
    public:

        /// @brief Constructs zero or more instances of the ElevationFlagger.
        /// The flagger is responsible for reading the "parset" and constructing
        /// zero or more instances of itself, depending on the configuration.
        static vector< boost::shared_ptr<IFlagger> > build(
                const LOFAR::ParameterSet& parset,
                const casa::MeasurementSet& ms);

        /// @brief Constructor
        ElevationFlagger(const LOFAR::ParameterSet& parset);

        /// @see IFlagger::processRow()
        virtual void processRow(casa::MSColumns& msc, const casa::uInt row,
                                const bool dryRun);

        /// @see IFlagger::stats()
        virtual FlaggingStats stats(void) const;

    private:

        // Elevations are cached in "itsAntennaElevations" for a given timestamp
        // (itsTimeElevCalculated). This method updates the
        void updateElevations(casa::MSColumns& msc, const casa::uInt row);

        // Utility method to flag the current row. Both the ROWFLAG and FLAG
        // data are set.
        void flagRow(casa::MSColumns& msc, const casa::uInt row, const bool dryRun);

        // Flagging statistics
        FlaggingStats itsStats;

        // Flagging threshold. If the elevation of an antenna is
        // larger than this then the row will be flagged.
        casa::Quantity itsHighLimit;

        // Flagging threshold. If the elevation of an antenna is
        // less than this then the row will be flagged.
        casa::Quantity itsLowLimit;

        // Timestamp that the antenna elevations vector was updated
        casa::Double itsTimeElevCalculated;

        // Antenna elevations, as calculated at time "itsTimeElevCalculated"
        casa::Vector<casa::Quantity> itsAntennaElevations;
};

}
}
}

#endif
