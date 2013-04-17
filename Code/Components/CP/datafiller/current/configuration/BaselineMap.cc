/// @file BaselineMap.cc
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

// Include own header file first
#include "BaselineMap.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <map>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "Common/ParameterSet.h"
#include "measures/Measures/Stokes.h"

// Using
using namespace askap;
using namespace askap::utility;
using namespace askap::cp;
using namespace askap::cp::ingest;
using namespace casa;

BaselineMap::BaselineMap(const LOFAR::ParameterSet& parset)
{
    const vector<uint32_t> ids = parset.getUint32Vector("baselineids", true);
    itsSize = ids.size();

    for (vector<uint32_t>::const_iterator it = ids.begin();
            it != ids.end(); ++it) {
        const uint32_t id = *it;
        if (!parset.isDefined(toString(id))) {
            ASKAPTHROW(AskapError, "Baseline mapping for id " << id << " not present");
        }

        const vector<string> tuple = parset.getStringVector(toString(id));
        if (tuple.size() != 3) {
            ASKAPTHROW(AskapError, "Baseline mapping for id " << id << " is malformed");
        }

        itsAntenna1Map[id] = fromString<uint32_t>(tuple[0]); 
        itsAntenna2Map[id] = fromString<uint32_t>(tuple[1]); 
        itsStokesMap[id] = Stokes::type(tuple[2]);
    }
    ASKAPCHECK(itsAntenna1Map.size() == itsSize, "Antenna 1 Map is of invalid size");
    ASKAPCHECK(itsAntenna2Map.size() == itsSize, "Antenna 2 Map is of invalid size");
    ASKAPCHECK(itsStokesMap.size() == itsSize, "Stokes type map is of invalid size");
}

int32_t BaselineMap::idToAntenna1(const uint32_t id) const
{
    std::map<int32_t, int32_t>::const_iterator it = itsAntenna1Map.find(id);
    if (it != itsAntenna1Map.end()) {
        return it->second;
    } else {
        return 1;
    }
}

int32_t BaselineMap::idToAntenna2(const uint32_t id) const
{
    std::map<int32_t, int32_t>::const_iterator it = itsAntenna2Map.find(id);
    if (it != itsAntenna2Map.end()) {
        return it->second;
    } else {
        return 1;
    }
}

casa::Stokes::StokesTypes BaselineMap::idToStokes(const uint32_t id) const
{
    std::map<int32_t, Stokes::StokesTypes>::const_iterator it = itsStokesMap.find(id);
    if (it != itsStokesMap.end()) {
        return it->second;
    } else {
        return Stokes::Undefined;
    }
}

size_t BaselineMap::size() const
{
    return itsSize;
}
