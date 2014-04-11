/// @file Antenna.cc
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

// Include own header file first
#include "Antenna.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/Quanta.h"
#include "casa/BasicSL.h"
#include "casa/Arrays/Vector.h"
#include "measures/Measures/MPosition.h"
#include "measures/Measures/MCPosition.h"
#include <measures/Measures/MeasConvert.h>

ASKAP_LOGGER(logger, ".Antenna");

using namespace askap;
using namespace askap::cp::ingest;
using namespace casa;

Antenna::Antenna(const casa::String& name,
                 const casa::String& mount,
                 const casa::Vector<casa::Double>& position,
                 const casa::Quantity& diameter)
        : itsName(name), itsMount(mount), itsPosition(position),
        itsDiameter(diameter)
{
    ASKAPCHECK(itsDiameter.isConform("m"),
               "Diameter must conform to meters");
    ASKAPCHECK(position.nelements() == 3,
               "Position vector must have three elements");
}

casa::String Antenna::name(void) const
{
    return itsName;
}

casa::String Antenna::mount(void) const
{
    return itsMount;
}

casa::Vector<casa::Double> Antenna::position(void) const
{
    return itsPosition;
}

casa::Quantity Antenna::diameter(void) const
{
    return itsDiameter;
}

casa::Vector<casa::Double> Antenna::convertAntennaPosition(const std::vector<double>& wgs84)
{
    const size_t LEN = 3;
    ASKAPCHECK(wgs84.size() == LEN, "Position vector must be of length 3");
    const MPosition pos(MVPosition(Quantity(wgs84[2], "m"),
                Quantity(wgs84[0], "deg"),
                Quantity(wgs84[1], "deg")),
            MPosition::Ref(MPosition::WGS84));
    const MPosition itrf = MPosition::Convert(pos, MPosition::ITRF)();
    const MVPosition itrfValue = itrf.getValue();

    casa::Vector<casa::Double> out(LEN);
    for (size_t i = 0; i < LEN; ++i) {
        out[i] = itrfValue.getValue()[i];
    }

    return out;

}
