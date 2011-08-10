/// @file GenericSolution.h
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

#ifndef ASKAP_CP_CALDATASERVICE_GENERICSOLUTION_H
#define ASKAP_CP_CALDATASERVICE_GENERICSOLUTION_H

// System includes
#include <map>
#include <vector>

// ASKAPsoft includes
#include "casa/aipstype.h"

// Local package includes
#include "accessprs/JonesJTerm.h"
#include "accessors/JonesDTerm.h"
#include "accessors/JonesIndex.h"

namespace askap {
namespace accessors {

// Calibration solution template
template<class T>
class GenericSolution {

    public:
        /// Constructor
        ///
        /// @param[in] timestamp    timestamp indicating when the solution was
        ///                         created. Absolute time expressed as
        ///                         microseconds since MJD=0.
        GenericSolution(const casa::Long timestamp) {
            itsTimestamp = timestamp;
        }

        /// Returns the timestamp for when the solution was created.
        /// @return the timestamp.
        casa::Long timestamp(void) const
        {
            return itsTimestamp;
        }

        /// Returns a const reference to the map containing the
        /// calibration parameters.
        const std::map<JonesIndex, T>& map(void) const
        {
            return itsMap;
        }

        /// Returns a non-const reference to the map containing the
        /// calibration parameters.
        std::map<JonesIndex, T>& map(void)
        {
            return itsMap;
        }

    private:
        casa::Long itsTimestamp;

        std::map<JonesIndex, T> itsMap;
};

/// Template instance for the Gain Solution
typedef GenericSolution<JonesJTerm> GainSolution;

/// Template instance for the Leakage Solution
typedef GenericSolution<JonesDTerm> LeakageSolution;

/// Template instance for the Bandpass Solution
typedef GenericSolution< std::vector<JonesJTerm> > BandpassSolution;

};
};

#endif
