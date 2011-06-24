/// @file Observation.cc
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
#include "Observation.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"

// Local package includes
#include "configuration/Scan.h"

using namespace askap;
using namespace askap::cp::ingest;

Observation::Observation(const casa::uInt schedulingBlockID,
                         const std::vector<Scan>& scans)
        : itsSchedulingBlockID(schedulingBlockID), itsScans(scans)
{
    ASKAPCHECK(!scans.empty(), "Observation must have one or more scans");
}

casa::uInt Observation::schedulingBlockID(void) const
{
    return itsSchedulingBlockID;
}

std::vector<Scan> Observation::scans(void) const
{
    return itsScans;
}
