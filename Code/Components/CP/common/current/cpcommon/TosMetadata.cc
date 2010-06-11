/// @file TosMetadata.cc
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
#include "TosMetadata.h"

// System includes
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"

// Using
using namespace askap::cp;

TosMetadata::TosMetadata(const casa::uInt& nCoarseChannels,
        const casa::uInt& nBeams,
        const casa::uInt& nPol) :
    itsNumCoarseChannels(nCoarseChannels),
    itsNumBeams(nBeams),
    itsNumPol(nPol),
    itsTime(0),
    itsPeriod(0)
{
}

casa::uInt TosMetadata::nAntenna(void) const
{
    return itsAntenna.size();
}

casa::uInt TosMetadata::nCoarseChannels(void) const
{
    return itsNumCoarseChannels;
}

casa::uInt TosMetadata::nBeams(void) const
{
    return itsNumBeams;
}

casa::uInt TosMetadata::nPol(void) const
{
    return itsNumPol;
}

casa::uLong TosMetadata::time(void) const
{
    return itsTime;
}

casa::uLong TosMetadata::period(void) const
{
    return itsPeriod;
}

void TosMetadata::time(const casa::uLong& time)
{
    itsTime = time;
}

void TosMetadata::period(const casa::uLong& period)
{
    itsPeriod = period;
}

casa::uInt TosMetadata::addAntenna(const casa::String& name)
{
    // Ensure an antenna of this name does not already exist
    std::vector<TosMetadataAntenna>::const_iterator it;
    for (it = itsAntenna.begin(); it != itsAntenna.end(); ++it) {
        if (it->name() == name) {
            ASKAPTHROW(AskapError, "An antenna with this name already exists");
        }
    }

    TosMetadataAntenna antenna(name, itsNumCoarseChannels,
            itsNumBeams, itsNumPol);

    itsAntenna.push_back(antenna);
    return (itsAntenna.size() - 1);
}

const TosMetadataAntenna& TosMetadata::antenna(const casa::uInt id) const
{
    checkAntennaId(id);
    return itsAntenna[id];
}

TosMetadataAntenna& TosMetadata::antenna(const casa::uInt id)
{
    checkAntennaId(id);
    return itsAntenna[id];
}

void TosMetadata::checkAntennaId(const casa::uInt& id) const
{
    if ((id + 1) > (nAntenna())) {
        ASKAPTHROW(AskapError, "Invalid antenna index");
    }
}
