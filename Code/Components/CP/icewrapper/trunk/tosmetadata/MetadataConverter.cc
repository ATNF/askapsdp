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

// Using
using namespace askap::cp;
using namespace askap::interfaces;
using namespace casa;

askap::cp::TosMetadata MetadataConverter::convert(const askap::interfaces::TimeTaggedTypedValueMap& source)
{
    // Use a mapper to easily get access to the elements and map them
    // to native types
    TypedValueMapMapper mapper(source.data);

    // First need to determine the number of beams, coarse channels and
    // polarisations before the askap::cp::TosMetadata object can be
    // instantiated
    const casa::Int nCoarseChan = mapper.getInt("n_coarse_chan");
    const std::vector<casa::Int> nBeam = mapper.getIntSeq("n_beams");
    const casa::Int nPol = mapper.getInt("n_pol");

    TosMetadata dest(nCoarseChan, nBeam[0], nPol);
    return dest;
}

askap::interfaces::TimeTaggedTypedValueMap MetadataConverter::convert(const askap::cp::TosMetadata& source)
{
    TimeTaggedTypedValueMap dest;
    dest.timestamp = source.time();

    // For indexing into array, matrices and cubes
    const casa::uInt nCoarseChan = source.nCoarseChannels();
    const casa::uInt nBeam = source.nBeams();
    const casa::uInt nPol = source.nPol();
    const casa::uInt nAntenna = source.nAntenna();

    // time
    dest.data["time"] = new TypedValueLong(TypeLong, source.time());

    // period
    dest.data["period"] = new TypedValueLong(TypeLong, source.period());

    // n_coarse_chan
    dest.data["n_coarse_chan"] = new TypedValueInt(TypeInt, nCoarseChan);

    // n_antennas
    dest.data["n_antennas"] = new TypedValueInt(TypeInt, nAntenna);

    // n_beams
    {
        IntSeq iseq;
        iseq.assign(nCoarseChan, nBeam);
        TypedValueIntSeqPtr tv = new TypedValueIntSeq(TypeIntSeq, iseq);
        dest.data["n_beams"] = tv;
    }

    // n_pol
    dest.data["n_pol"] = new TypedValueInt(TypeInt, nPol);

    // antenna_names
    {
        StringSeq sseq;

        for (unsigned int i = 0; i < nAntenna; ++i) {
            sseq.push_back(source.antenna(i).name());
        }

        TypedValueStringSeqPtr tv = new TypedValueStringSeq(TypeStringSeq, sseq);
        dest.data["antenna_names"] = tv;
    }

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
    // Instance of TosMetadataAntenna to convert
    const TosMetadataAntenna& antenna = source.antenna(antId);
    const std::string antennaName = antenna.name();

    // For indexing into array, matrices and cubes
    const casa::uInt nCoarseChan = source.nCoarseChannels();
    const casa::uInt nBeam = source.nBeams();
    const casa::uInt nPol = source.nPol();

    // <antenna name>.dish_pointing
    Direction dishPointing;
    dishPointing.coord1 = antenna.dishPointing().getAngle().getValue()(0);
    dishPointing.coord2 = antenna.dishPointing().getAngle().getValue()(1);
    dishPointing.sys = J2000;
    dest.data[makeMapKey(antennaName, "dish_pointing")] =
        new TypedValueDirection(TypeDirection, dishPointing);

    // <antenna name>.frequency
    dest.data[makeMapKey(antennaName, "frequency")] =
        new TypedValueDouble(TypeDouble, antenna.frequency());

    // <antenna name>.client_id
    dest.data[makeMapKey(antennaName, "client_id")] =
        new TypedValueString(TypeString, antenna.clientId());

    // <antenna name>.scan_id
    dest.data[makeMapKey(antennaName, "scan_id")] =
        new TypedValueString(TypeString, antenna.scanId());

    // <antenna name>.phase_tracking_centre
    DirectionSeq ptc;
    ptc.resize(nBeam * nCoarseChan);

    for (unsigned int beam = 0; beam < nBeam; ++beam) {
        for (unsigned int coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
            casa::MDirection srcDirection = antenna.phaseTrackingCentre(beam, coarseChan);
            const int idx = beam + ((nBeam) * coarseChan);
            ptc[idx].coord1 = srcDirection.getAngle().getValue()(0);
            ptc[idx].coord2 = srcDirection.getAngle().getValue()(1);
            ptc[idx].sys = J2000;
        }
    }
    dest.data[makeMapKey(antennaName, "phase_tracking_centre")] =
        new TypedValueDirectionSeq(TypeDirectionSeq, ptc);

    // <antenna name>.parallactic_angle
    dest.data[makeMapKey(antennaName, "parallactic_angle")] =
        new TypedValueDouble(TypeDouble, antenna.parallacticAngle());

    // <antenna name>.flag.on_source
    dest.data[makeMapKey(antennaName, "flag.on_source")] =
        new TypedValueBool(TypeBool, antenna.onSource());

    // <antenna name>.flag.hw_error
    dest.data[makeMapKey(antennaName, "flag.hw_error")] =
        new TypedValueBool(TypeBool, antenna.hwError());

    // <antenna name>.flag.detailed
    BoolSeq flag(nBeam * nCoarseChan * nPol);
    for (unsigned int beam = 0; beam < nBeam; ++beam) {
        for (unsigned int coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
            for (unsigned int pol = 0; pol < nPol; ++pol) {
                const unsigned int idx = beam + ((nBeam) * coarseChan) +
                    ((nBeam * nCoarseChan) * pol);
                flag[idx] = antenna.flagDetailed(beam, coarseChan, pol);
            }
        }
    }
    dest.data[makeMapKey(antennaName, "flag.detailed")] = new TypedValueBoolSeq(TypeBoolSeq, flag);

    // <antenna name>.system_temp
    FloatSeq systemTemp(nBeam * nCoarseChan * nPol);
    for (unsigned int beam = 0; beam < nBeam; ++beam) {
        for (unsigned int coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
            for (unsigned int pol = 0; pol < nPol; ++pol) {
                const unsigned int idx = beam + ((nBeam) * coarseChan) +
                    ((nBeam * nCoarseChan) * pol);
                systemTemp[idx] = antenna.systemTemp(beam, coarseChan, pol);
            }
        }
    }
    dest.data[makeMapKey(antennaName, "system_temp")] =
        new TypedValueFloatSeq(TypeFloatSeq, systemTemp);
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
