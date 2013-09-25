/// @file MetadataConverter.cc
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
#include "MetadataConverter.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "boost/shared_ptr.hpp"
#include "cpcommon/TosMetadata.h"
#include "casa/aips.h"

// CP Ice interfaces
#include "TypedValues.h"

// Local package includes
#include "TypedValueMapMapper.h"
#include "TypedValueMapConstMapper.h"

// Using
using namespace askap::cp::icewrapper;
using namespace askap::interfaces;
using namespace casa;

askap::cp::TosMetadata MetadataConverter::convert(const askap::interfaces::TimeTaggedTypedValueMap& source)
{
    // Use a mapper to easily get access to the elements and map them
    // to native (or casa) types
    TypedValueMapConstMapper srcMapper(source.data);

    // A copy of this object will be returned from this method
    TosMetadata dest;

    // time
    //dest.time(srcMapper.getLong("timestamp"));
    dest.time(source.timestamp);

    // period
    dest.scanId(srcMapper.getInt("scan_id"));

    // user_flag
    dest.flagged(srcMapper.getBool("user_flag"));

    // antenna_names
    const std::vector<casa::String> antennaNames = srcMapper.getStringSeq("antennas");

    /////////////////////////
    // Metadata per antenna
    /////////////////////////
    for (size_t i = 0; i < antennaNames.size(); ++i) {
        convertAntenna(antennaNames[i], source, dest);
    }

    return dest;
}

askap::interfaces::TimeTaggedTypedValueMap MetadataConverter::convert(const askap::cp::TosMetadata& source)
{
    TimeTaggedTypedValueMap dest;
    dest.timestamp = source.time();

    // Use a mapper to easily convert native (or casa types) to TypedValues
    TypedValueMapMapper destMapper(dest.data);

    // scan_id
    destMapper.setInt("scan_id", source.scanId());

    // user_flag
    destMapper.setBool("user_flag", source.flagged());

    // antenna_names
    std::vector<casa::String> antennaNames;
    for (size_t i = 0; i < source.nAntennas(); ++i) {
        antennaNames.push_back(source.antenna(i).name());
    }
    destMapper.setStringSeq("antennas", antennaNames);

    /////////////////////////
    // Metadata per antenna
    /////////////////////////
    for (unsigned int i = 0; i < source.nAntennas(); ++i) {
        convertAntenna(i, source, dest);
    }

    return dest;
}

// Convert antenna portion of the Tos Metadata from
// askap::cp::TosMetadata to askap::interfaces::TimeTaggedTypedValueMap
void MetadataConverter::convertAntenna(unsigned int antId, 
        const askap::cp::TosMetadata& source, 
        askap::interfaces::TimeTaggedTypedValueMap& dest)
{
    TypedValueMapMapper destMapper(dest.data);

    // Obtain the instance of TosMetadataAntenna to convert
    const TosMetadataAntenna& antenna = source.antenna(antId);
    const std::string antennaName = antenna.name();

    // <antenna name>.actual_radec
    destMapper.setDirection(makeMapKey(antennaName, "actual_radec"),
            antenna.actualRaDec());

    // <antenna name>.actual_azel
    destMapper.setDirection(makeMapKey(antennaName, "actual_azel"),
            antenna.actualAzEl());

    // <antenna name>.actual_pol
    destMapper.setFloat(makeMapKey(antennaName, "actual_pol"),
            antenna.actualPolAngle().getValue("rad"));

    // <antenna name>.on_source
    destMapper.setBool(makeMapKey(antennaName, "on_source"),
            antenna.onSource());

    // <antenna name>.flagged
    destMapper.setBool(makeMapKey(antennaName, "flagged"),
            antenna.hwError());
}

// Convert antenna portion of the Tos Metadata from
// askap::interfaces::TimeTaggedTypedValueMap to askap::cp::TosMetadata
void MetadataConverter::convertAntenna(const std::string& antennaName,
        const askap::interfaces::TimeTaggedTypedValueMap& source,
        askap::cp::TosMetadata& dest)
{
    // Use a mapper to easily get access to the elements and map them
    // to native (or casa) types
    TypedValueMapConstMapper srcMapper(source.data);

    const unsigned int id = dest.addAntenna(antennaName);
    TosMetadataAntenna& ant = dest.antenna(id);

    // hw_error
    ant.hwError(srcMapper.getBool(makeMapKey(antennaName,
                    "flagged")));

    // If the antenna is flagged (other than for being !on_source then the other
    // metadata may not be present
    if (!ant.hwError()) {
        // actual_radec
        ant.actualRaDec(srcMapper.getDirection(makeMapKey(antennaName,
                        "actual_radec")));
        // actual_azel
        ant.actualAzEl(srcMapper.getDirection(makeMapKey(antennaName,
                        "actual_azel")));
        // actual_pol
        ant.actualPolAngle(Quantity(srcMapper.getFloat(makeMapKey(antennaName,
                        "actual_pol")), "rad"));
        // on_source
        ant.onSource(srcMapper.getBool(makeMapKey(antennaName,
                        "on_source")));
    }
}

std::string MetadataConverter::makeMapKey(const std::string& prefix,
        const std::string& suffix)
{
    std::ostringstream ss;
    ss << prefix << "." << suffix;
    return ss.str();
}
