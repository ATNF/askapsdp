/// @file Scan.cc
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
#include "Scan.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/BasicSL.h"
#include "casa/Quanta.h"
#include "measures/Measures/MDirection.h"

using namespace askap;
using namespace askap::cp::ingest;

Scan::Scan(const casa::String& fieldName,
           const casa::MDirection& fieldDirection,
           const casa::Quantity& startFreq,
           const casa::String& correlatorMode)
        : itsFieldName(fieldName), itsFieldDirection(fieldDirection),
        itsCentreFreq(startFreq), itsCorrelatorMode(correlatorMode)
{
    ASKAPCHECK(startFreq.isConform("Hz"),
            "Centre frequency must conform to Hz");
}

casa::String Scan::name(void) const
{
    return itsFieldName;
}

casa::MDirection Scan::fieldDirection(void) const
{
    return itsFieldDirection;
}

casa::Quantity Scan::startFreq(void) const
{
    return itsCentreFreq;
}

casa::String Scan::correlatorMode(void) const
{
    return itsCorrelatorMode;
}
