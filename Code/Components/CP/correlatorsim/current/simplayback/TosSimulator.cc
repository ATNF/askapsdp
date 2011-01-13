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
    itsPort.reset(new askap::cp::icewrapper::MetadataOutputPort(locatorHost, locatorPort,
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
    const casa::ROMSSpWindowColumns& spwc = msc.spectralWindow();
    const casa::ROMSDataDescColumns& ddc = msc.dataDescription();
    const casa::ROMSPolarizationColumns& polc = msc.polarization();
    //const casa::ROMSPointingColumns& pointingc = msc.pointing();

    // Define some useful variables
    const casa::Int dataDescId = msc.dataDescId()(itsCurrentRow);
    const casa::uInt descPolId = ddc.polarizationId()(dataDescId);
    const casa::uInt descSpwId = ddc.spectralWindowId()(dataDescId);
    const casa::uInt nRow = msc.nrow(); // In the whole table, not just for this integration
    const casa::uInt nCorr = polc.numCorr()(descPolId);
    //const casa::uInt nChan = spwc.numChan()(descSpwId);
    const casa::uInt nAntenna = antc.nrow();
    const casa::uInt nBeam = feedc.nrow() / nAntenna;
    const casa::uInt nCoarseChan = 304;

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

    // time and period
    // Note: The time read from the measurement set is the integration midpoint,
    // while the TOS metadata specification calls for the integration start time.
    // Hence the below conversion.
    const casa::Long Tmid = static_cast<long>(currentIntegration * 1000 * 1000);
    const casa::Long Tint = static_cast<long>(msc.interval()(itsCurrentRow) * 1000 * 1000);
    const casa::Long Tstart = Tmid - (Tint / 2);
    metadata.time(Tstart);
    metadata.period(Tint);


    ////////////////////////////////////////
    // Metadata - per antenna
    ////////////////////////////////////////
    for (casa::uInt i = 0; i < nAntenna; ++i) {
        const std::string name = antc.name().getColumn()(i);

        const casa::uInt id = metadata.addAntenna(name);
        TosMetadataAntenna& antMetadata = metadata.antenna(id);

        const casa::Int fieldId = msc.fieldId()(itsCurrentRow);
        const casa::Vector<casa::MDirection> dirVec = fieldc.phaseDirMeasCol()(fieldId);
        const casa::MDirection direction = dirVec(0);

        // <antenna name>.target_radec
        antMetadata.targetRaDec(direction);

        // <antenna name>.frequency
        // TODO: This is actually just the start frequency, not the centre
        // frequency so it breaks the interface contract. Anyway, it is ignored
        // by the central processor.
        casa::Vector<casa::Double> chanFreq = spwc.chanFreq()(descSpwId);
        antMetadata.frequency(chanFreq(0));

        // <antenna name>.client_id
        antMetadata.clientId("N/A");

        // <antenna name.scan_active
        antMetadata.scanActive(true);

        // <antenna name>.scan_id
        std::ostringstream ss;
        ss << msc.scanNumber()(itsCurrentRow);
        antMetadata.scanId(ss.str());

        // <antenna name>.phase_tracking_centre
        for (casa::uInt beam = 0; beam < nBeam; ++beam) {
            antMetadata.phaseTrackingCentre(direction, beam);
        }

        // <antenna name>.polarisation_offset 
        // TODO: Zero is ok for data coming from the csimulator when an
        // equatorial mount is simulated.
        antMetadata.polarisationOffset(0.0);

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
        // TODO: The csimulator does not write a SYSCAL table henece no system
        // temperature is available. It would be nice to perhaps read this
        // table if it does exist.
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
        // The TOM has no way of indicating end of scan, but it does set the
        // scan_active field to false when the observation is complete and
        // guarantees at least one metadata payload will be sent with 
        // scan_active == false. So here an additional metadata message is sent
        // indicating this idle state.
        for (casa::uInt i = 0; i < nAntenna; ++i) {
            TosMetadataAntenna& antMetadata = metadata.antenna(i);
            metadata.time(metadata.time() + metadata.period());
            antMetadata.scanActive(false);
            antMetadata.scanId("");
            antMetadata.clientId("");
        }
        ASKAPLOG_INFO_STR(logger,
                "Sending additional metadata message indicating end-of-observation");
        itsPort->send(metadata);

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
