/// @file BaselineMapper.h
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

namespace askap {
namespace cp {
namespace ingest {

/// @brief
class BaselineMap {
    public:
        BaselineMap(const LOFAR::ParameterSet& parset);

        int32_t idToAntenna1(const uint32_t id) const;
        int32_t idToAntenna2(const uint32_t id) const;
        casa::Stokes::StokesTypes idToStokes(const uint32_t id) const;
        size_t size() const;

        /// @brief obtain largest id
        /// @details This is required to initialise a flat array buffer holding
        /// derived per-id information because the current implementation does not
        /// explicitly prohibits sparse ids.
        /// @return the largest id setup in the map
        uint32_t maxID() const;

        /// @brief find an id matching baseline/polarisation description
        /// @details This is the reverse look-up operation.
        /// @param[in] ant1 index of the first antenna
        /// @param[in] ant2 index of the second antenna
        /// @param[in] pol polarisation product
        /// @return the index of the selected baseline/polarisation
        /// @note an exception is thrown if there is no match
        uint32_t getID(const int32_t ant1, const int32_t ant2, const casa::Stokes::StokesTypes pol) const;

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
