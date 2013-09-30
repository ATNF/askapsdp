/// @file BaselineMap.h
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

#ifndef ASKAP_CP_INGEST_BASELINEMAP_H
#define ASKAP_CP_INGEST_BASELINEMAP_H

// System include
#include <map>
#include <stdint.h>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "measures/Measures/Stokes.h"
#include "askap/AskapError.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief Maps the baseline id, as is supplied in the VisDatagram by the
/// Correlator IOC, to a pair of antennas and a correlation product.
///
/// Below is the complete entry for an example 3-antenna system:
/// <PRE>
/// baselinemap.baselineids            = [1..21]
///
/// baselinemap.1                      = [0, 0, XX]
/// baselinemap.2                      = [0, 0, XY]
/// baselinemap.3                      = [0, 1, XX]
/// baselinemap.4                      = [0, 1, XY]
/// baselinemap.5                      = [0, 2, XX]
/// baselinemap.6                      = [0, 2, XY]
/// baselinemap.7                      = [0, 0, YY]
/// baselinemap.8                      = [0, 1, YX]
/// baselinemap.9                      = [0, 1, YY]
/// baselinemap.10                     = [0, 2, YX]
/// baselinemap.11                     = [0, 2, YY]
///
/// baselinemap.12                     = [1, 1, XX]
/// baselinemap.13                     = [1, 1, XY]
/// baselinemap.14                     = [1, 2, XX]
/// baselinemap.15                     = [1, 2, XY]
/// baselinemap.16                     = [1, 1, YY]
/// baselinemap.17                     = [1, 2, YX]
/// baselinemap.18                     = [1, 2, YY]
///
/// baselinemap.19                     = [2, 2, XX]
/// baselinemap.20                     = [2, 2, XY]
/// baselinemap.21                     = [2, 2, YY]
/// </PRE>
class BaselineMap {
    public:

        /// @brief Constructor.
        /// @param[in] parset   a parset (i.e. a map from string to string)
        ///     describing the range of entries and the contents of the entries.
        ///     An example is shown in the class comments.
        BaselineMap(const LOFAR::ParameterSet& parset);

        /// Given a baseline is, return antenna 1
        ///
        /// @param[in] id   the baseline id.
        /// @return antennaid, or -1 in the case the baseline id mapping
        ///         does not exist.
        int32_t idToAntenna1(const int32_t id) const;

        /// Given a baseline is, return antenna 2
        ///
        /// @param[in] id   the baseline id.
        /// @return antennaid, or -1 in the case the baseline id mapping
        ///         does not exist.
        int32_t idToAntenna2(const int32_t id) const;

        /// Given a baseline is, return the stokes type
        ///
        /// @param[in] id   the baseline id.
        /// @return stokes type, or Stokes::Undefined in the case the baseline
        ///         id mapping does not exist.
        casa::Stokes::StokesTypes idToStokes(const int32_t id) const;

        /// Returns the number of entries in the map
        size_t size() const;

        /// @brief obtain largest id
        /// @details This is required to initialise a flat array buffer holding
        /// derived per-id information because the current implementation does not
        /// explicitly prohibits sparse ids.
        /// @return the largest id setup in the map
        int32_t maxID() const;

        /// @brief find an id matching baseline/polarisation description
        /// @details This is the reverse look-up operation.
        /// @param[in] ant1 index of the first antenna
        /// @param[in] ant2 index of the second antenna
        /// @param[in] pol polarisation product
        /// @return the index of the selected baseline/polarisation, or -1 if
        ///         the selected baseline/polarisation does not exist in the map.
        int32_t getID(const int32_t ant1, const int32_t ant2, const casa::Stokes::StokesTypes pol) const;

    private:

        size_t itsSize;
        std::map<int32_t, int32_t> itsAntenna1Map;
        std::map<int32_t, int32_t> itsAntenna2Map;
        std::map<int32_t, casa::Stokes::StokesTypes> itsStokesMap;
};


};
};
};
#endif
