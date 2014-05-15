/// @file Antenna.h
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

#ifndef ASKAP_CP_INGEST_ANTENNA_H
#define ASKAP_CP_INGEST_ANTENNA_H

// System includes
#include <vector>

// ASKAPsoft includes
#include "casa/aips.h"
#include "casa/Quanta.h"
#include "casa/BasicSL.h"
#include "casa/Arrays/Vector.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief This class encapsulates an antenna.
class Antenna {
    public:

        /// @brief Constructor
        Antenna(const casa::String& name,
                const casa::String& mount,
                const casa::Vector<casa::Double>& position,
                const casa::Quantity& diameter);

        /// @brief Antenna name (e.g. "ak01")
        casa::String name(void) const;

        // @brief Mount type of antenna (e.g. "EQUATORIAL", "ALT-AZ")
        casa::String mount(void) const;

        /// @brief Positin of the antenna. This position is in the right-handed frame, X towards
        /// the intersection of the equator and the Greenwich meridian, Z towards the pole.
        casa::Vector<casa::Double> position(void) const;

        /// @brief Nominal diameter of the dish.
        casa::Quantity diameter(void) const;

    private:
        casa::String itsName;
        casa::String itsMount;
        casa::Vector<casa::Double> itsPosition;
        casa::Quantity itsDiameter;
};

}
}
}

#endif
