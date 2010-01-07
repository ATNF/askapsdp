/// @file MSReader.cc
///
/// @copyright (c) 2009 CSIRO
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
#include "MSReader.h"

// Include package level header file
#include <askap_correlatorsim.h>

// System includes
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "cpcommon/VisPayload.h"

// Local package includes
#include "cpinterfaces/CommonTypes.h"
#include "cpinterfaces/TypedValues.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;
using namespace casa;

ASKAP_LOGGER(logger, ".MSReader");

MSReader::MSReader(const std::string& filename)
: itsMS(filename, casa::Table::Old), itsCurrentRow(0)
{
}

MSReader::~MSReader()
{
}

bool MSReader::fillNext(askap::interfaces::TimeTaggedTypedValueMap& metadata,
        std::vector<askap::cp::VisPayload>& visVec)
{
    ROMSColumns msc(itsMS);

    // Get a reference to the columns of interest
    const casa::ROMSAntennaColumns &antc = msc.antenna();
    const casa::ROMSFeedColumns &feedc = msc.feed();
    //const casa::ROMSFieldColumns &fieldc = msc.field();
    //const casa::ROMSSpWindowColumns &spwc = msc.spectralWindow();
    const casa::ROMSDataDescColumns &ddc = msc.dataDescription();
    const casa::ROMSPolarizationColumns &polc = msc.polarization();

    // Define some useful variables
    const int dataDescId = msc.dataDescId()(itsCurrentRow);
    const unsigned int descPolId = ddc.polarizationId()(dataDescId);
    //const unsigned int descSpwId = ddc.spectralWindowId()(dataDescId);
    const unsigned int nRow = msc.nrow(); // In the whole table, not just for this integration
    const unsigned int nCorr = polc.numCorr()(descPolId);
    //const unsigned int nChan = spwc.numChan()(descSpwId);
    const unsigned int nAntenna = antc.nrow();
    const unsigned int nBeam = feedc.nrow() / nAntenna;
    //const unsigned int nBaseline = (nAntenna * (nAntenna + 1)) / 2;
    const unsigned int nCoarseChan = 1;

    // Record the timestamp for the current integration that is
    // being processed
    casa::Double currentIntegration = msc.time()(itsCurrentRow);
    ASKAPLOG_DEBUG_STR(logger, "Processing integration with timestamp "
            << std::setprecision (13) << currentIntegration);


    //////////////////////////////////////////////////////////////
    // Metadata
    //////////////////////////////////////////////////////////////

    // time
    {
        const long timestamp = static_cast<long>(currentIntegration * 1000 * 1000);
        metadata.timestamp = timestamp;
        metadata.data["time"] = new TypedValueLong(TypeLong, timestamp);
    }

    // period
    {
        const long interval = static_cast<long>(msc.interval()(itsCurrentRow) * 1000 * 1000);
        metadata.data["period"] = new TypedValueLong(TypeLong, interval);
    }

    // n_coarse_chan
    {
        metadata.data["n_coarse_chan"] = new TypedValueInt(TypeInt, nCoarseChan);
    }

    // n_antennas 
    {
        metadata.data["n_antennas"] = new TypedValueInt(TypeInt, nAntenna);
    }

    // n_beams
    {
        IntSeq iseq;
        iseq.assign(nCoarseChan, nBeam);
        TypedValueIntSeqPtr tv = new TypedValueIntSeq(TypeIntSeq, iseq);
        metadata.data["n_beams"] = tv;
    }

    // n_pol
    {
        metadata.data["n_pol"] = new TypedValueInt(TypeInt, nCorr);
    }

    // antenna_names 
    {
        casa::Vector<casa::String> v  = antc.name().getColumn();
        StringSeq sseq;
        for (unsigned int i = 0; i < v.size(); ++i) {
            sseq.push_back(v(i));
        }
        TypedValueStringSeqPtr tv = new TypedValueStringSeq(TypeStringSeq, sseq);
        metadata.data["antenna_names"] = tv;
    }

    ////////////////////////////////////////
    // Metadata - per antenna
    ////////////////////////////////////////
    for (unsigned int i = 0; i < nAntenna; ++i) {
        const std::string name = antc.name().getColumn()(i);

        {
            // <antenna name>.dish_pointing
            Direction dishPointing;
            dishPointing.coord1 = 0.0; // TODO
            dishPointing.coord2 = 0.0; // TODO
            dishPointing.sys = J2000;
            metadata.data[makeMapKey(name, "dish_pointing")] = new TypedValueDirection(TypeDirection, dishPointing);
        }

        {
            // <antenna name>.frequency
            metadata.data[makeMapKey(name, "frequency")] = new TypedValueDouble(TypeDouble, 0.0); // TODO
        }

        {
            // <antenna name>.client_id
            const std::string clientId = "N/A";
            metadata.data[makeMapKey(name, "client_id")] = new TypedValueString(TypeString, clientId);
        }

        {
            // <antenna name>.scan_id
            const std::string scanId = "0";
            metadata.data[makeMapKey(name, "scan_id")] = new TypedValueString(TypeString, scanId);
        }

        {
            // <antenna name>.phase_tracking_centre
            DirectionSeq ptc;
            ptc.resize(nBeam * nCoarseChan);
            // TODO
            metadata.data[makeMapKey(name, "phase_tracking_centre")] = new TypedValueDirectionSeq(TypeDirectionSeq, ptc);
        }

        {
            // <antenna name>.parallactic_angle
            metadata.data[makeMapKey(name, "parallactic_angle")] = new TypedValueDouble(TypeDouble, 0.0); // TODO
        }

        {
            // <antenna name>.flag.on_source
            metadata.data[makeMapKey(name, "flag.on_source")] = new TypedValueBool(TypeBool, true); // TODO
        }

        {
            // <antenna name>.flag.hw_error
            metadata.data[makeMapKey(name, "flag.hw_error")] = new TypedValueBool(TypeBool, true); // TODO
        }

        {
            // <antenna name>.flag.detailed
            BoolSeq flag;
            flag.resize(nBeam * nCoarseChan * nCorr);
            // TODO
            metadata.data[makeMapKey(name, "flag.detailed")] = new TypedValueBoolSeq(TypeBoolSeq, flag);
        }

        {
            // <antenna name>.system_temp
            FloatSeq systemTemp;
            systemTemp.resize(nBeam * nCoarseChan * nCorr);
            // TODO
            metadata.data[makeMapKey(name, "system_temp")] = new TypedValueFloatSeq(TypeFloatSeq, systemTemp);
        }
    }

    ////////////////////////////////////////
    // Visibilities
    ////////////////////////////////////////

    // Process rows until none are left or the timestamp
    // changes, indicating the end of this integration
    while (itsCurrentRow != nRow && (currentIntegration == msc.time()(itsCurrentRow))) {
        // This code needs the dataDescId to remain constant for all rows
        // in the integration being processed
        ASKAPCHECK(msc.dataDescId()(itsCurrentRow) == dataDescId,
                "Data description ID must remain constant for a given integration");
        VisPayload payload;
        visVec.push_back(payload);
        itsCurrentRow++;
    }

    if (itsCurrentRow == nRow) {
        return false; // Indicate there is no more data after this payload
    } else {
        return true;
    }
}

std::string MSReader::makeMapKey(const std::string &prefix, const std::string &suffix)
{
    std::ostringstream ss;
    ss << prefix << "." << suffix;
    return ss.str();
}
