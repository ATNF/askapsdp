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

#ifndef ASKAP_CP_SIMPLAYBACK_BASELINEMAP_H
#define ASKAP_CP_SIMPLAYBACK_BASELINEMAP_H

// System include
#include <map>
#include <stdint.h>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "measures/Measures/Stokes.h"

namespace askap {
namespace cp {

/// @brief
class BaselineMap {
    public:
        BaselineMap(const LOFAR::ParameterSet& parset);

        int32_t operator()(int32_t antenna1, int32_t antenna2, const casa::Stokes::StokesTypes& stokes) const;

    private:

        struct BaselineMapKey
        {
            int32_t antenna1;
            int32_t antenna2;
            casa::Stokes::StokesTypes stokes;

            bool operator<(const BaselineMapKey& rhs) const
            {
                if (antenna1 < rhs.antenna1) return true;
                if (antenna1 > rhs.antenna1) return false;
                if (antenna2 < rhs.antenna2) return true;
                if (antenna2 > rhs.antenna2) return false;
                if (stokes < rhs.stokes) return true;
                if (stokes > rhs.stokes) return false;
                return false;
            }
        };

        std::map<BaselineMapKey, int32_t> itsMap;
};

};
};
#endif
