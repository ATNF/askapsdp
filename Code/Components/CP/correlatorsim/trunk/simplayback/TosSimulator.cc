/// @file TosSimulator.cc
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
#include "TosSimulator.h"

// Include package level header file
#include "askap_correlatorsim.h"

// System includes
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "casa/Arrays/Vector.h"
#include "measures/Measures/MDirection.h"
#include "tosmetadata/MetadataOutputPort.h"

// ICE interface includes
#include "CommonTypes.h"
#include "TypedValues.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;
using namespace casa;

ASKAP_LOGGER(logger, ".TosSimulator");

TosSimulator::TosSimulator(const std::string& dataset,
        const std::string& locatorHost,
        const std::string& locatorPort,
        const std::string& topicManager,                
        const std::string& topic)
: itsCurrentRow(0)
{
    itsMS.reset(new casa::MeasurementSet(dataset, casa::Table::Old));
    itsPort.reset(new askap::cp::MetadataOutputPort(locatorHost, locatorPort,
                  topicManager, topic));
}

TosSimulator::~TosSimulator()
{
    itsMS.reset();
    itsPort.reset();
}

bool TosSimulator::sendNext(void)
{
    ROMSColumns msc(*itsMS);

    // Get a reference to the columns of interest
    const casa::ROMSAntennaColumns& antc = msc.antenna();
    const casa::ROMSFeedColumns& feedc = msc.feed();
    const casa::ROMSFieldColumns& fieldc = msc.field();
    //const casa::ROMSSpWindowColumns& spwc = msc.spectralWindow();
    const casa::ROMSDataDescColumns& ddc = msc.dataDescription();
    const casa::ROMSPolarizationColumns& polc = msc.polarization();
    //const casa::ROMSPointingColumns& pointingc = msc.pointing();

    // Define some useful variables
    const int dataDescId = msc.dataDescId()(itsCurrentRow);
    const unsigned int descPolId = ddc.polarizationId()(dataDescId);
    //const unsigned int descSpwId = ddc.spectralWindowId()(dataDescId);
    const unsigned int nRow = msc.nrow(); // In the whole table, not just for this integration
    const unsigned int nCorr = polc.numCorr()(descPolId);
    //const unsigned int nChan = spwc.numChan()(descSpwId);
    const unsigned int nAntenna = antc.nrow();
    const unsigned int nBeam = feedc.nrow() / nAntenna;
    const unsigned int nCoarseChan = 304;

    // Record the timestamp for the current integration that is
    // being processed
    casa::Double currentIntegration = msc.time()(itsCurrentRow);
    ASKAPLOG_DEBUG_STR(logger, "Processing integration with timestamp "
                           << std::setprecision(13) << currentIntegration);


    //////////////////////////////////////////////////////////////
    // Metadata
    //////////////////////////////////////////////////////////////

    // Some constraints
    ASKAPCHECK(fieldc.nrow() == 1, "Currently only support a single field");

    // Initialize the metadata message
    askap::cp::TosMetadata metadata(nCoarseChan, nBeam, nCorr);

    // time
    const long timestamp = static_cast<long>(currentIntegration * 1000 * 1000);
    metadata.time(timestamp);

    // period
    const long interval = static_cast<long>(msc.interval()(itsCurrentRow) * 1000 * 1000);
    metadata.period(interval);


    ////////////////////////////////////////
    // Metadata - per antenna
    ////////////////////////////////////////
    for (unsigned int i = 0; i < nAntenna; ++i) {
        const std::string name = antc.name().getColumn()(i);

        const casa::uInt id = metadata.addAntenna(name);
        TosMetadataAntenna& antMetadata = metadata.antenna(id);

        const int fieldId = msc.fieldId()(itsCurrentRow);
        const casa::Vector<casa::MDirection> dirVec = fieldc.phaseDirMeasCol()(fieldId);
        const casa::MDirection direction = dirVec(0);

        // <antenna name>.dish_pointing
        antMetadata.dishPointing(direction);

        // <antenna name>.frequency
        // TODO: Currently this is ignored by the CP, but if possible it would be good
        // to use the correct figure
        antMetadata.frequency(0.0);

        // <antenna name>.client_id
        antMetadata.clientId("N/A");

        // <antenna name>.scan_id
        antMetadata.scanId("0");

        // <antenna name>.phase_tracking_centre
        for (casa::uInt coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
            for (casa::uInt beam = 0; beam < nBeam; ++beam) {
                antMetadata.phaseTrackingCentre(direction, beam, coarseChan);
            }
        }

        // <antenna name>.parallactic_angle
        // TODO: Should not be zero.
        antMetadata.parallacticAngle(0.0);

        // <antenna name>.flag.on_source
        // TODO: Current no flagging, but it would be good to read this from the
        // actual measurement set
        antMetadata.onSource(true);

        // <antenna name>.flag.hw_error
        // TODO: Current no flagging, but it would be good to read this from the
        // actual measurement set
        antMetadata.hwError(false);

        // <antenna name>.flag.detailed
        // TODO: Current no flagging, but it would be good to read this from the
        // actual measurement set
        for (casa::uInt coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
            for (casa::uInt beam = 0; beam < nBeam; ++beam) {
                for (casa::uInt pol = 0; pol < nCorr; ++pol) {
                    antMetadata.flagDetailed(false, beam, coarseChan, pol);
                }
            }
        }

        // <antenna name>.system_temp
        // TODO: Current no system temperature, but it would be good to read this
        // from the actual measurement set
        for (casa::uInt coarseChan = 0; coarseChan < nCoarseChan; ++coarseChan) {
            for (casa::uInt beam = 0; beam < nBeam; ++beam) {
                for (casa::uInt pol = 0; pol < nCorr; ++pol) {
                    antMetadata.systemTemp(0.0, beam, coarseChan, pol);
                }
            }
        }
    }

    // Find the end of the current integration (i.e. find the next timestamp)
    // or the end of the table
    while (itsCurrentRow != nRow && (currentIntegration == msc.time()(itsCurrentRow))) {
        itsCurrentRow++;
    }

    // Send the payload
    itsPort->send(metadata);

    if (itsCurrentRow == nRow) {
        return false; // Indicate there is no more data after this payload
    } else {
        return true;
    }
}

std::string TosSimulator::makeMapKey(const std::string &prefix, const std::string &suffix)
{
    std::ostringstream ss;
    ss << prefix << "." << suffix;
    return ss.str();
}
