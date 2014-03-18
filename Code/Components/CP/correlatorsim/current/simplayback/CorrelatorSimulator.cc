/// @file CorrelatorSimulator.cc
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
#include "CorrelatorSimulator.h"

// Include package level header file
#include "askap_correlatorsim.h"

// System includes
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <inttypes.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/Stokes.h"
#include "cpcommon/VisDatagram.h"

// casa
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCEpoch.h>


// Local package includes
#include "simplayback/BaselineMap.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace casa;

ASKAP_LOGGER(logger, ".CorrelatorSimulator");

CorrelatorSimulator::CorrelatorSimulator(const std::string& dataset,
        const std::string& hostname,
        const std::string& port,
        const BaselineMap& bmap,
        const unsigned int expansionFactor,
        const double visSendFail,
        const int shelf)
    : itsBaselineMap(bmap), itsExpansionFactor(expansionFactor),
        itsVisSendFailChance(visSendFail), itsShelf(shelf),
        itsCurrentRow(0), itsRandom(0.0, 1.0)
{
    if (expansionFactor > 1) {
        ASKAPLOG_DEBUG_STR(logger, "Using expansion factor of " << expansionFactor);
    } else {
        ASKAPLOG_DEBUG_STR(logger, "No expansion factor");
    }
    itsMS.reset(new casa::MeasurementSet(dataset, casa::Table::Old));
    itsPort.reset(new askap::cp::VisPort(hostname, port));
}

CorrelatorSimulator::~CorrelatorSimulator()
{
    itsMS.reset();
    itsPort.reset();
}

bool CorrelatorSimulator::sendNext(void)
{
    ROMSColumns msc(*itsMS);

    // Get a reference to the columns of interest
    //const casa::ROMSAntennaColumns& antc = msc.antenna();
    //const casa::ROMSFeedColumns& feedc = msc.feed();
    const casa::ROMSFieldColumns& fieldc = msc.field();
    const casa::ROMSSpWindowColumns& spwc = msc.spectralWindow();
    const casa::ROMSDataDescColumns& ddc = msc.dataDescription();
    const casa::ROMSPolarizationColumns& polc = msc.polarization();
    //const casa::ROMSPointingColumns& pointingc = msc.pointing();
    const unsigned int nRow = msc.nrow(); // In the whole table, not just for this integration
 
    // Record the timestamp for the current integration that is
    // being processed
    const casa::Double currentIntegration = msc.time()(itsCurrentRow);
    ASKAPLOG_DEBUG_STR(logger, "Processing integration with timestamp "
            << msc.timeMeas()(itsCurrentRow));

    // Some general constraints
    ASKAPCHECK(fieldc.nrow() == 1, "Currently only support a single field");

    // Counts the number of simulated (randomised) failures to send
    // visibilities this cycle
    unsigned long failureCount = 0;

    ////////////////////////////////////////
    // Visibilities
    ////////////////////////////////////////

    // Process rows until none are left or the timestamp
    // changes, indicating the end of this integration
    while (itsCurrentRow != nRow && (currentIntegration == msc.time()(itsCurrentRow))) {

        // Define some useful variables
        const int dataDescId = msc.dataDescId()(itsCurrentRow);
        const unsigned int descPolId = ddc.polarizationId()(dataDescId);
        const unsigned int descSpwId = ddc.spectralWindowId()(dataDescId);
        const unsigned int nCorr = polc.numCorr()(descPolId);
        const unsigned int nChan = spwc.numChan()(descSpwId);
        //const unsigned int nAntenna = antc.nrow();
        //const unsigned int nBeam = feedc.nrow() / nAntenna;


        // Some per row constraints
        // This code needs the dataDescId to remain constant for all rows
        // in the integration being processed
        ASKAPCHECK(msc.dataDescId()(itsCurrentRow) == dataDescId,
                "Data description ID must remain constant for a given integration");

        // Populate the VisDatagram
        askap::cp::VisDatagram payload;
        payload.version = VISPAYLOAD_VERSION;

        // Note, the measurement set stores integration midpoint (in seconds), while the TOS
        // (and it is assumed the correlator) deal with integration start (in microseconds)
        // In addition, TOS time is BAT and the measurement set normally has UTC time
        // (the latter is not checked here as we work with the column as a column of doubles
        // rather than column of measures)
        
        // precision of a single double may not be enough in general, but should be fine for 
        // this emulator (ideally need to represent time as two doubles)
        const casa::MEpoch epoch(casa::MVEpoch(casa::Quantity(currentIntegration,"s")), 
                                 casa::MEpoch::Ref(casa::MEpoch::UTC));
        const casa::MVEpoch epochTAI = casa::MEpoch::Convert(epoch,
                               casa::MEpoch::Ref(casa::MEpoch::TAI))().getValue();
        const uint64_t microsecondsPerDay = 86400000000ull;
        const uint64_t startOfDayBAT = uint64_t(epochTAI.getDay()*microsecondsPerDay);
        const long Tint = static_cast<long>(msc.interval()(itsCurrentRow) * 1000 * 1000);
        const uint64_t startBAT = startOfDayBAT + uint64_t(epochTAI.getDayFraction()*microsecondsPerDay) -
                                  uint64_t(Tint / 2);

        // ideally we need to carry 64-bit BAT in the payload explicitly
        payload.timestamp = static_cast<long>(startBAT);
        ASKAPCHECK(msc.feed1()(itsCurrentRow) == msc.feed2()(itsCurrentRow),
                "feed1 and feed2 must be equal");

        // NOTE: The Correlator IOC uses one-based beam indexing, so need to add
        // one to the zero-based indexes from the measurement set.
        payload.beamid = msc.feed1()(itsCurrentRow) + 1;

        // Get the expansion factor, producing the actual number of channels
        // to simulate
        const unsigned int nChanActual = itsExpansionFactor * nChan;

        // Calculate how many slices to send to encompass all channels
        const unsigned int nSlices = nChanActual / N_CHANNELS_PER_SLICE;
        ASKAPCHECK(nChanActual % N_CHANNELS_PER_SLICE == 0,
                "Number of channels must be divisible by N_CHANNELS_PER_SLICE");

        // TODO: Below, the slice starts at zero for each process where only
        // rank zero should start at slice zero. Rank 1 will start at some
        // offset. Fix this in future.

        // This matrix is: Matrix<Complex> data(nCorr, nChan)
        const casa::Matrix<casa::Complex> data = msc.data()(itsCurrentRow);
        const int antenna1 = msc.antenna1()(itsCurrentRow);
        const int antenna2 = msc.antenna2()(itsCurrentRow);
        const casa::Vector<casa::Int> stokesTypesInt = polc.corrType()(descPolId);

        for (unsigned int corr = 0; corr < nCorr; ++corr) {
            const Stokes::StokesTypes stokestype = Stokes::type(stokesTypesInt(corr));
            ASKAPCHECK(stokestype == Stokes::XX ||
                    stokestype == Stokes::XY ||
                    stokestype == Stokes::YX ||
                    stokestype == Stokes::YY,
                   "Unsupported stokes type");

            // The ASKAP correlator does not send both XY and YX for auto-correlations
            // so mimic this behaviour here
            if ((antenna1 == antenna2) && (stokestype == Stokes::YX)) {
                continue;
            }

            const int32_t baselineid = itsBaselineMap(antenna1, antenna2, stokestype);
            if (baselineid < 0) {
                ASKAPLOG_WARN_STR(logger, "Baseline ID does not exist for - ant1: "
                        << antenna1 << ", ant2: " << antenna2 << ", Corr: " << Stokes::name(stokestype));
                continue;
            }
            payload.baselineid = baselineid;
            unsigned int sliceOffset;
            if (itsShelf == 1) {
                sliceOffset = 0;
            } else if (itsShelf == 2) {
                sliceOffset = 8;
            } else {
                ASKAPTHROW(AskapError, "No support for more than two shelves yet");
            }
            for (unsigned int slice = 0; slice < nSlices; ++slice) {
                payload.slice = slice + sliceOffset;
                for (unsigned int chan = 0; chan < N_CHANNELS_PER_SLICE; ++chan) {
                    const unsigned int offset = static_cast<unsigned int>(
                            ceil(((slice * N_CHANNELS_PER_SLICE) + chan) / itsExpansionFactor));
                    payload.vis[chan].real = data(corr, offset).real();
                    payload.vis[chan].imag = data(corr, offset).imag();
                }
                // Finished populating, send this payload but then reuse it in the
                // next iteration of the loop for the next packet

                // Use a RNG to simulate random failure to send packets
                if (itsRandom.gen() > itsVisSendFailChance) {
                    itsPort->send(payload);
                } else {
                    ++failureCount;
                }

                // Sleep for a while to smooth the packet flow. This is an
                // arbitary time suited to sending BETA scale datasets, and should
                // be updated in future to be more general (TODO)
                usleep(50);
            }
        }

        itsCurrentRow++;
    }

    if (itsVisSendFailChance > 0.0) {
        ASKAPLOG_DEBUG_STR(logger, "Randomly failed to send " << failureCount << " payloads this cycle");
    }

    if (itsCurrentRow == nRow) {
        return false; // Indicate there is no more data after this payload
    } else {
        return true;
    }
}
