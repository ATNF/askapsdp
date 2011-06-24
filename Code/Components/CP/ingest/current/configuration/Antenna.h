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

// ASKAPsoft includes
#include "casa/aips.h"
#include "casa/Quanta.h"
#include "casa/BasicSL.h"
#include "casa/Arrays/Vector.h"

// Local package includes
#include "configuration/FeedConfig.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief TODO: Write documentation...
class Antenna {
    public:

        /// @brief Constructor
        Antenna(const casa::String& name,
                const casa::String& mount,
                const casa::Vector<casa::Double>& position,
                const casa::Quantity& diameter,
                const FeedConfig& feeds);

        casa::String name(void) const;

        casa::String mount(void) const;

        casa::Vector<casa::Double> position(void) const;

        casa::Quantity diameter(void) const;

        FeedConfig feeds(void) const;

    private:
        const casa::String itsName;
        const casa::String itsMount;
        const casa::Vector<casa::Double> itsPosition;
        const casa::Quantity itsDiameter;
        const FeedConfig itsFeeds;

};

}
}
}

#endif
