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
#include "askap_correlatorsim.h"

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
using namespace casa;

BaselineMap::BaselineMap(const LOFAR::ParameterSet& parset)
{
    const vector<uint32_t> ids = parset.getUint32Vector("baselineids", true);

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

        BaselineMapKey bkey;
        bkey.antenna1 = fromString<uint32_t>(tuple[0]);
        bkey.antenna2 = fromString<uint32_t>(tuple[1]);
        bkey.stokes = Stokes::type(tuple[2]);

        itsMap[bkey] = id;
    }
}

int32_t BaselineMap::operator()(int32_t antenna1, int32_t antenna2, const casa::Stokes::StokesTypes& stokes) const
{
    BaselineMapKey key;
    key.antenna1 = antenna1;
    key.antenna2 = antenna2;
    key.stokes = stokes;

    std::map<BaselineMapKey, int32_t>::const_iterator it = itsMap.find(key);
    if (it == itsMap.end()) {
        return -1;
    } else {
        return it->second;
    }
}
