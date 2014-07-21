/// @file FeedConfig.cc
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
#include "FeedConfig.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/BasicSL.h"
#include "casa/Quanta.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"

// Local package includes

ASKAP_LOGGER(logger, ".FeedConfig");

using namespace askap;
using namespace askap::cp::ingest;

FeedConfig::FeedConfig(const casa::Matrix<casa::Quantity>& offsets,
                       const casa::Vector<casa::String>& pols) :
        itsOffsets(offsets), itsPols(pols)
{
    ASKAPCHECK(offsets.ncolumn() == 2,
               "Offset matrix should have two columns");
    ASKAPCHECK(offsets.nrow() > 0,
               "Offsets should have at least one row");
    ASKAPCHECK(offsets.nrow() == pols.nelements(),
               "shape of offsets matrix and polarisations vector not consistent");

    // Ensure all offsets conform to radians
    casa::Matrix<casa::Quantity>::const_iterator it;

    for (it = itsOffsets.begin(); it != itsOffsets.end(); ++it) {
        ASKAPCHECK(it->isConform("rad"), "Offset must conform to radians");
    }
}

casa::Quantity FeedConfig::offsetX(casa::uInt i) const
{
    ASKAPCHECK(i < itsOffsets.nrow(),
               "Feed index out of bounds");
    return itsOffsets(i, 0);
}

casa::Quantity FeedConfig::offsetY(casa::uInt i) const
{
    ASKAPCHECK(i < itsOffsets.nrow(),
               "Feed index out of bounds");
    return itsOffsets(i, 1);
}

casa::String FeedConfig::pol(casa::uInt i) const
{
    ASKAPCHECK(i < itsPols.nelements(),
               "Feed index out of bounds");
    return itsPols(i);
}

casa::uInt FeedConfig::nFeeds(void) const
{
    return itsOffsets.nrow();
}
