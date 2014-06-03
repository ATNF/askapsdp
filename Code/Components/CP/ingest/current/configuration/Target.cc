/// @file Target.cc
///
/// @copyright (c) 2014 CSIRO
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
#include "Target.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/BasicSL.h"
#include "measures/Measures/MDirection.h"

using namespace askap;
using namespace askap::cp::ingest;

Target::Target(const casa::String& name,
        const casa::MDirection& pointingCentre,
        const casa::MDirection& phaseCentre,
        const CorrelatorMode& mode)
: itsName(name), itsPointingCentre(pointingCentre),
   itsPhaseCentre(phaseCentre), itsMode(mode)
{
}

casa::String Target::name(void) const
{
    return itsName;
}

casa::MDirection Target::pointingCentre(void) const
{
    return itsPointingCentre;
}

casa::MDirection Target::phaseCentre(void) const
{
    return itsPhaseCentre;
}

const CorrelatorMode& Target::mode(void) const
{
    return itsMode;
}
