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
using namespace askap::cp;
using namespace askap::interfaces;
using namespace casa;

askap::cp::TosMetadata MetadataConverter::convert(const askap::interfaces::TimeTaggedTypedValueMap& source)
{
    // Use a mapper to easily get access to the elements and map them
    // to native (or casa) types
    TypedValueMapConstMapper srcMapper(source.data);

    // First need to determine the number of beams, coarse channels and
    // polarisations before the askap::cp::TosMetadata object can be
    // instantiated
    const casa::Int nCoarseChan = srcMapper.getInt("n_coarse_chan");
    const std::vector<casa::Int> nBeam = srcMapper.getIntSeq("n_beams");
    const casa::Int nPol = srcMapper.getInt("n_pol");
    const casa::Int nAntenna = srcMapper.getInt("n_antennas");

    TosMetadata dest(nCoarseChan, nBeam[0], nPol);

    // time
    dest.time(srcMapper.getLong("time"));

    /////////////////////////
    // Metadata per antenna
    /////////////////////////
    for (int i = 0; i < nAntenna; ++i) {
        convertAntenna(i, source, dest);
    }

    return dest;
}

askap::interfaces::TimeTaggedTypedValueMap MetadataConverter::convert(const askap::cp::TosMetadata& source)
{
    TimeTaggedTypedValueMap dest;

    // Use a mapper to easily convert native (or casa types) to TypedValues
    TypedValueMapMapper destMapper(dest.data);

    dest.timestamp = source.time();

    // For indexing into array, matrices and cubes
    const casa::uInt nCoarseChan = source.nCoarseChannels();
    const casa::uInt nBeam = source.nBeams();
    const casa::uInt nPol = source.nPol();
    const casa::uInt nAntenna = source.nAntenna();

    // time
    destMapper.setLong("time", source.time());

    // period
    destMapper.setLong("period", source.period());

    // n_coarse_chan
    destMapper.setInt("n_coarse_chan", nCoarseChan);

    // n_antennas
    destMapper.setInt("n_antennas", nAntenna);

    // n_beams
    std::vector<casa::Int> nBeamsVec;
    nBeamsVec.assign(nCoarseChan, nBeam);
    destMapper.setIntSeq("n_beams", nBeamsVec);

    // n_pol
    destMapper.setInt("n_pol", nPol);

    // antenna_names
    std::vector<casa::String> antennaNames;
    for (unsigned int i = 0; i < nAntenna; ++i) {
        antennaNames.push_back(source.antenna(i).name());
    }
    destMapper.setStringSeq("antenna_names", antennaNames);

    /////////////////////////
    // Metadata per antenna
    /////////////////////////
    for (unsigned int i = 0; i < nAntenna; ++i) {
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

    // For indexing into array, matrices and cubes
    const casa::uInt nCoarseChan = source.nCoarseChannels();
    const casa::uInt nBeam = source.nBeams();
    const casa::uInt nPol = source.nPol();

    // <antenna name>.dish_pointing
    destMapper.setDirection(makeMapKey(antennaName, "dish_pointing"),
            antenna.dishPointing());

    // <antenna name>.frequency
    destMapper.setDouble(makeMapKey(antennaName, "frequency"),
            antenna.frequency());

    // <antenna name>.client_id
    destMapper.setString(makeMapKey(antennaName, "client_id"),
            antenna.clientId());

    // <antenna name>.scan_id
    destMapper.setString(makeMapKey(antennaName, "scan_id"),
            antenna.scanId());

    // <antenna name>.phase_tracking_centre
    // Need to convert this Matrix to a 1D array with appropriate indexing
    std::vector<casa::MDirection> ptcVector(nBeam * nCoarseChan);
    for (unsigned int beam = 0; beam < nBeam; ++beam) {
        for (unsigned int coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
            const int idx = beam + ((nBeam) * coarseChan);
            ptcVector[idx] = antenna.phaseTrackingCentre(beam, coarseChan);
        }
    }
    destMapper.setDirectionSeq(makeMapKey(antennaName, "phase_tracking_centre"),
            ptcVector);

    // <antenna name>.parallactic_angle
    destMapper.setDouble(makeMapKey(antennaName, "parallactic_angle"),
            antenna.parallacticAngle());

    // <antenna name>.flag.on_source
    destMapper.setBool(makeMapKey(antennaName, "flag.on_source"),
            antenna.onSource());

    // <antenna name>.flag.hw_error
    destMapper.setBool(makeMapKey(antennaName, "flag.hw_error"),
            antenna.hwError());

    // <antenna name>.flag.detailed
    // Need to convert this Cube to a 1D array with appropriate indexing
    std::vector<casa::Bool> flagVector(nBeam * nCoarseChan * nPol);
    for (unsigned int beam = 0; beam < nBeam; ++beam) {
        for (unsigned int coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
            for (unsigned int pol = 0; pol < nPol; ++pol) {
                const unsigned int idx = beam + ((nBeam) * coarseChan) +
                                            ((nBeam * nCoarseChan) * pol);
                flagVector[idx] = antenna.flagDetailed(beam, coarseChan, pol);
            }
        }
    }
    destMapper.setBoolSeq(makeMapKey(antennaName, "flag.detailed"),
            flagVector);

    // <antenna name>.system_temp
    // Need to convert this Cube to a 1D array with appropriate indexing
    std::vector<casa::Float> systemTempVec(nBeam * nCoarseChan * nPol);
    for (unsigned int beam = 0; beam < nBeam; ++beam) {
        for (unsigned int coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
            for (unsigned int pol = 0; pol < nPol; ++pol) {
                const unsigned int idx = beam + ((nBeam) * coarseChan) +
                                            ((nBeam * nCoarseChan) * pol);
                systemTempVec[idx] = antenna.systemTemp(beam, coarseChan, pol);
            }
        }
    }
    destMapper.setFloatSeq(makeMapKey(antennaName, "system_temp"),
            systemTempVec);
}

// Convert antenna portion of the Tos Metadata from
// askap::interfaces::TimeTaggedTypedValueMap to askap::cp::TosMetadata
void MetadataConverter::convertAntenna(unsigned int antId,
        const askap::interfaces::TimeTaggedTypedValueMap& source,
        askap::cp::TosMetadata& dest)
{
}

std::string MetadataConverter::makeMapKey(const std::string& prefix,
        const std::string& suffix)
{
    std::ostringstream ss;
    ss << prefix << "." << suffix;
    return ss.str();
}
