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
#include <map>
#include <utility>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/Quanta.h"

// Using
using namespace std;
using namespace askap::cp;

TosMetadata::TosMetadata() : itsTime(0), itsScanId(-1), itsFlagged(false)
{
}

casa::uLong TosMetadata::time(void) const
{
    return itsTime;
}

void TosMetadata::time(const casa::uLong time)
{
    itsTime = time;
}

casa::Int TosMetadata::scanId(void) const
{
    return itsScanId;
}

void TosMetadata::scanId(const casa::Int id)
{
    itsScanId = id;
}

casa::Bool TosMetadata::flagged(void) const
{
    return itsFlagged;
}

void TosMetadata::flagged(const casa::Bool flag)
{
    itsFlagged = flag;
}

casa::Quantity TosMetadata::centreFreq(void) const
{
    return itsCentreFreq;
}

void TosMetadata::centreFreq(const casa::Quantity& freq)
{
    itsCentreFreq = freq;
}

void TosMetadata::addAntenna(const TosMetadataAntenna& ant)
{
    // Ensure an antenna of this name does not already exist
    const map<string, TosMetadataAntenna>::const_iterator it = itsAntennas.find(ant.name());
    if (it != itsAntennas.end()) {
        ASKAPTHROW(AskapError, "An antenna with this name (" << ant.name()
                << ") already exists");
    }

    itsAntennas.insert(make_pair(ant.name(), ant));
}

casa::uInt TosMetadata::nAntenna() const
{
    return static_cast<casa::uInt>(itsAntennas.size());
}

std::vector<std::string> TosMetadata::antennaNames(void) const
{
    vector<string> names;
    for (map<string, TosMetadataAntenna>::const_iterator it = itsAntennas.begin();
            it != itsAntennas.end(); ++it) {
        names.push_back(it->first);
    }
    return names;
}

const TosMetadataAntenna& TosMetadata::antenna(const std::string& name) const
{
    const map<string, TosMetadataAntenna>::const_iterator it = itsAntennas.find(name);
    if (it == itsAntennas.end()) {
        ASKAPTHROW(AskapError, "Antenna " << name << " not found in metadata");
    }
    return it->second;
}
