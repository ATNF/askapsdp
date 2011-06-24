/// @file CorrelatorMode.cc
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
#include "CorrelatorMode.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include "vector"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/BasicSL.h"
#include "casa/Quanta.h"
#include "measures/Measures/Stokes.h"

using namespace askap;
using namespace askap::cp::ingest;

CorrelatorMode::CorrelatorMode(const casa::String& name,
                               const casa::uInt nChan,
                               const casa::Quantity chanWidth,
                               const std::vector<casa::Stokes::StokesTypes> stokes)
        : itsName(name), itsNChan(nChan),
        itsChanWidth(chanWidth), itsStokes(stokes)
{
    ASKAPCHECK(chanWidth.isConform("Hz"),
               "Channel width must conform to Hz");
    ASKAPCHECK(!stokes.empty(), "Stokes vector is empty");
}

casa::String CorrelatorMode::name(void) const
{
    return itsName;
}

casa::uInt CorrelatorMode::nChan(void) const
{
    return itsNChan;
}

casa::Quantity CorrelatorMode::chanWidth(void) const
{
    return itsChanWidth;
}

std::vector<casa::Stokes::StokesTypes> CorrelatorMode::stokes(void) const
{
    return itsStokes;
}
