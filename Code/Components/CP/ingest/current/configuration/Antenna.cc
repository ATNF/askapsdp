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

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/Quanta.h"
#include "casa/BasicSL.h"
#include "measures/Measures/MPosition.h"

// Local package includes
#include "configuration/FeedConfig.h"

ASKAP_LOGGER(logger, ".Antenna");

using namespace askap;
using namespace askap::cp::ingest;

Antenna::Antenna(const casa::String& name,
                 const casa::String& mount,
                 const casa::Vector<casa::Double>& position,
                 const casa::Quantity& diameter,
                 const FeedConfig& feeds)
        : itsName(name), itsMount(mount), itsPosition(position),
        itsDiameter(diameter), itsFeeds(feeds)
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

FeedConfig Antenna::feeds(void) const
{
    return itsFeeds;
}
