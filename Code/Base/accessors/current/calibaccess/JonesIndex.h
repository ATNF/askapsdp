/// @file JonesIndex.h
///
/// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_CP_CALDATASERVICE_JONESINDEX_H
#define ASKAP_CP_CALDATASERVICE_JONESINDEX_H

// ASKAPsoft includes
#include "casa/aipstype.h"

namespace askap {
namespace accessors {

/// Key type used for indexing into the calibration solution maps for the
/// GainSolution, LeakageSolution and BandpassSolution classes.
class JonesIndex {

    public:
        /// Constructor
        ///
        /// @param[in] antenna  ID of the antenna. This must be the physical
        ///                     antenna ID.
        /// @param[in] beam     ID of the beam. Again, must map to an actual
        ///                     beam.
        JonesIndex(const casa::Short antenna, casa::Short beam);

        /// Obtain the antenna ID
        /// @return the antenna ID
        casa::Short antenna(void) const;

        /// Obtain the beam ID
        /// @return the beam ID
        casa::Short beam(void) const;

        /// Operator...
        bool operator==(const JonesIndex& rhs) const;

        /// Operator...
        bool operator!=(const JonesIndex& rhs) const;

        /// Operator...
        bool operator<(const JonesIndex& rhs) const;

    private:
        casa::Short itsAntenna;
        casa::Short itsBeam;
};

};
};

#endif
