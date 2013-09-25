/// @file TosMetadataAntenna.cc
///
/// @copyright (c) 2010 CSIRO
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
#include "TosMetadataAntenna.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "measures/Measures/MDirection.h"
#include "casa/Quanta/Quantum.h"

// Using
using namespace askap::cp;

TosMetadataAntenna::TosMetadataAntenna(const casa::String& name)
    : itsName(name), itsOnSource(false), itsHwError(true)
{
}

casa::String TosMetadataAntenna::name(void) const
{
    return itsName;
}

casa::MDirection TosMetadataAntenna::actualRaDec(void) const
{
    return itsActualRaDec;
}

void TosMetadataAntenna::actualRaDec(const casa::MDirection& val)
{
    itsActualRaDec = val;
}

casa::MDirection TosMetadataAntenna::actualAzEl(void) const
{
    return itsActualAzEl;
}

void TosMetadataAntenna::actualAzEl(const casa::MDirection& val)
{
    itsActualAzEl = val;
}

casa::Quantity TosMetadataAntenna::actualPolAngle(void) const
{
    return itsPolAngle;
}

void TosMetadataAntenna::actualPolAngle(const casa::Quantity& q)
{
    itsPolAngle = q;
}

casa::Bool TosMetadataAntenna::onSource(void) const
{
    return itsOnSource;
}

void TosMetadataAntenna::onSource(const casa::Bool& val)
{
    itsOnSource = val;
}

casa::Bool TosMetadataAntenna::hwError(void) const
{
    return itsHwError;
}

void TosMetadataAntenna::hwError(const casa::Bool& val)
{
    itsHwError = val;
}
