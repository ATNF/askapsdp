/// @file Target.h
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

#ifndef ASKAP_CP_INGEST_TARGET_H
#define ASKAP_CP_INGEST_TARGET_H

// System includes
#include <string>

// ASKAPsoft includes
#include "casa/BasicSL.h"
#include "measures/Measures/MDirection.h"

// Local package includes
#include "configuration/CorrelatorMode.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief This class encapsulates a "target"
class Target {
    public:

        /// @brief Constructor
        Target(const casa::String& name,
             const casa::MDirection& pointingCentre,
             const casa::MDirection& phaseCentre,
             const CorrelatorMode& mode);

        /// @brief Returns the name of the target field being observed
        casa::String name(void) const;

        /// @brief Returns the antenna pointing centre
        casa::MDirection pointingCentre(void) const;

        /// @brief Returns the phase centre
        casa::MDirection phaseCentre(void) const;

        /// @brief Correlator mode for this target
        const CorrelatorMode& mode(void) const;

    private:
        casa::String itsName;
        casa::MDirection itsPointingCentre;
        casa::MDirection itsPhaseCentre;
        CorrelatorMode itsMode;
};

}
}
}

#endif
