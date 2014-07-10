/// @file MergedSource.cc
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
#include "MergedSource.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <set>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"
#include "Common/ParameterSet.h"
#include "cpcommon/TosMetadata.h"
#include "cpcommon/VisDatagram.h"
#include "utils/PolConverter.h"
#include "casa/Quanta/MVEpoch.h"
#include "measures/Measures.h"
#include "measures/Measures/MeasFrame.h"
#include "measures/Measures/MCEpoch.h"
#include "measures/Measures/Stokes.h"

// Local package includes
#include "ingestpipeline/sourcetask/IVisSource.h"
#include "ingestpipeline/sourcetask/IMetadataSource.h"
#include "ingestpipeline/sourcetask/ScanManager.h"
#include "ingestpipeline/sourcetask/ChannelManager.h"
#include "ingestpipeline/sourcetask/InterruptedException.h"
#include "configuration/Configuration.h"
#include "configuration/BaselineMap.h"

ASKAP_LOGGER(logger, ".MergedSource");

using namespace std;
using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;

MergedSource::MergedSource(const LOFAR::ParameterSet& params,
        const Configuration& config,
        IMetadataSource::ShPtr metadataSrc,
        IVisSource::ShPtr visSrc, int numTasks, int id) :
     itsConfig(config),
     itsMetadataSrc(metadataSrc), itsVisSrc(visSrc),
     itsNumTasks(numTasks), itsId(id),
     itsScanManager(config),
     itsChannelManager(params),
     itsBaselineMap(config.bmap()),
     itsInterrupted(false),
     itsSignals(itsIOService, SIGINT, SIGTERM, SIGUSR1),
     itsNBeams(0),
     itsLastTimestamp(-1)
{
    // Trigger a dummy frame conversion with casa measures to ensure all caches are setup
    const casa::MVEpoch dummyEpoch(56000.);

    casa::MEpoch::Convert(casa::MEpoch(dummyEpoch, casa::MEpoch::Ref(casa::MEpoch::TAI)),
            casa::MEpoch::Ref(casa::MEpoch::UTC))();

    parseBeamMap(params);

    // Setup a signal handler to catch SIGINT, SIGTERM and SIGUSR1
    itsSignals.async_wait(boost::bind(&MergedSource::signalHandler, this, _1, _2));
}

MergedSource::~MergedSource()
{
    itsSignals.cancel();
}

VisChunk::ShPtr MergedSource::next(void)
{
    // Used for a timeout
    const long ONE_SECOND = 10000000;

    if (itsScanManager.scanIndex() < 0) {
        // If the TOS hasn't started the observation yet (i.e. scan id hasn't
        // changed from SCANID_IDLE), just eat metadata payloads until scan_id >= 0
        ASKAPLOG_INFO_STR(logger, "Waiting for first scan to begin");
        do {
            itsMetadata = itsMetadataSrc->next(ONE_SECOND);
            checkInterruptSignal();
            if (itsMetadata && itsMetadata->scanId() == ScanManager::SCANID_OBS_COMPLETE) {
                ASKAPLOG_WARN_STR(logger,
                        "Observation has been aborted before first scan was started");
                return VisChunk::ShPtr();
            }
        } while (!itsMetadata || itsMetadata->scanId() < 0);
    } else {
        do {
            itsMetadata = itsMetadataSrc->next(ONE_SECOND);
            checkInterruptSignal();
        } while (!itsMetadata);
    }

    // Update the Scan Manager
    itsScanManager.update(itsMetadata->scanId());

    // Check if the TOS/TOM has indicated the observation is complete
    if (itsScanManager.observationComplete()) {
        ASKAPLOG_INFO_STR(logger, "End-of-observation condition met");
        return VisChunk::ShPtr();
    }

    // Protect against producing VisChunks with the same timestamp
    ASKAPCHECK(itsMetadata->time() != itsLastTimestamp,
            "Consecutive VisChunks have the same timestamp");
    itsLastTimestamp = itsMetadata->time();

    // Get the next VisDatagram if there isn't already one in the buffer
    while (!itsVis) {
        itsVis = itsVisSrc->next(ONE_SECOND);
        checkInterruptSignal();
    }

    // Find data with matching timestamps
    bool logCatchup = true;
    while (itsMetadata->time() != itsVis->timestamp) {

        // If the VisDatagram timestamps are in the past (with respect to the
        // TosMetadata) then read VisDatagrams until they catch up
        while (!itsVis || itsMetadata->time() > itsVis->timestamp) {
            if (logCatchup) {
                ASKAPLOG_DEBUG_STR(logger, "Reading extra VisDatagrams to catch up");
                logCatchup = false;
            }
            itsVis = itsVisSrc->next(ONE_SECOND);

            checkInterruptSignal();
        }

        // But if the timestamp in the VisDatagram is in the future (with
        // respect to the TosMetadata) then it is time to fetch new TosMetadata
        if (!itsMetadata || itsMetadata->time() < itsVis->timestamp) {
            if (logCatchup) {
                ASKAPLOG_DEBUG_STR(logger, "Reading extra TosMetadata to catch up");
                logCatchup = false;
            }
            itsMetadata = itsMetadataSrc->next(ONE_SECOND);
            itsScanManager.update(itsMetadata->scanId());
            checkInterruptSignal();
            if (itsScanManager.observationComplete()) {
                ASKAPLOG_INFO_STR(logger, "End-of-observation condition met");
                return VisChunk::ShPtr();
            }
        }
    }

    // Now the streams are synced, start building a VisChunk
    VisChunk::ShPtr chunk = createVisChunk(*itsMetadata);

    // Determine how many VisDatagrams are expected for a single integration
    const casa::uInt nChannels = itsChannelManager.localNChannels(itsId);
    ASKAPCHECK(nChannels % N_CHANNELS_PER_SLICE == 0,
            "Number of channels must be divisible by N_CHANNELS_PER_SLICE");
    const casa::uInt datagramsExpected = itsBaselineMap.size() * itsNBeams * (nChannels / N_CHANNELS_PER_SLICE);
    const CorrelatorMode& mode = itsConfig.lookupCorrelatorMode(itsMetadata->corrMode());
    const casa::uInt interval = mode.interval();
    const casa::uInt timeout = interval * 2;

    // Read VisDatagrams and add them to the VisChunk. If itsVisSrc->next()
    // returns a null pointer this indicates the timeout has been reached.
    // In this case assume no more VisDatagrams for this integration will
    // be recieved and move on
    casa::uInt datagramCount = 0; 
    casa::uInt datagramsIgnored = 0;
    std::set<DatagramIdentity> receivedDatagrams;
    while (itsVis && itsMetadata->time() >= itsVis->timestamp) {
        checkInterruptSignal();

        if (itsMetadata->time() > itsVis->timestamp) {
            // If the VisDatagram is from a prior integration then discard it
            ASKAPLOG_WARN_STR(logger, "Received VisDatagram from past integration");
            itsVis = itsVisSrc->next(timeout);
            continue;
        }

        if (addVis(chunk, *itsVis, *itsMetadata, receivedDatagrams)) {
            ++datagramCount;
        } else {
            ++datagramsIgnored;
        }
        itsVis.reset();

        if (datagramCount == datagramsExpected) {
            // This integration is finished
            break;
        }
        itsVis = itsVisSrc->next(timeout);
    }

    ASKAPLOG_DEBUG_STR(logger, "VisChunk built with " << datagramCount <<
            " of expected " << datagramsExpected << " visibility datagrams");
    ASKAPLOG_DEBUG_STR(logger, "     - ignored " << datagramsIgnored
        << " successfully received datagrams");

    // Submit monitoring data
    itsMonitoringPointManager.submitPoint<int32_t>("PacketsLostCount",
            datagramsExpected - datagramCount);
    if (datagramsExpected != 0) {
        itsMonitoringPointManager.submitPoint<float>("PacketsLostPercent",
            (datagramsExpected - datagramCount) / static_cast<float>(datagramsExpected) * 100.);
    }
    itsMonitoringPointManager.submitMonitoringPoints(*chunk);

    itsMetadata.reset();
    return chunk;
}

VisChunk::ShPtr MergedSource::createVisChunk(const TosMetadata& metadata)
{
    const CorrelatorMode& corrMode = itsConfig.lookupCorrelatorMode(metadata.corrMode());
    const casa::uInt nAntenna = itsConfig.antennas().size();
    ASKAPCHECK(nAntenna > 0, "Must have at least one antenna defined");
    const casa::uInt nChannels = itsChannelManager.localNChannels(itsId);
    const casa::uInt nPol = corrMode.stokes().size();
    const casa::uInt nBaselines = nAntenna * (nAntenna + 1) / 2;
    const casa::uInt nRow = nBaselines * itsNBeams;
    const casa::uInt period = corrMode.interval(); // in microseconds

    VisChunk::ShPtr chunk(new VisChunk(nRow, nChannels, nPol, nAntenna));

    // Convert the time from integration start in microseconds to an
    // integration mid-point in seconds
    const uint64_t midpointBAT = static_cast<uint64_t>(metadata.time()) + (period / 2ull);
    chunk->time() = bat2epoch(midpointBAT).getValue();
  
    // Convert the interval from microseconds (long) to seconds (double)
    const casa::Double interval = period / 1000.0 / 1000.0;
    chunk->interval() = interval;

    // All visibilities get flagged as bad, then as the visibility data 
    // arrives they are unflagged
    chunk->flag() = true;
    chunk->visibility() = 0.0;

    // For now polarisation data is hardcoded.
    ASKAPCHECK(nPol == 4, "Only supporting 4 polarisation products");
    for (casa::uInt polIndex = 0; polIndex < nPol; ++polIndex) {
         // this way of creating the Stokes vectors ensures the canonical order of polarisation products
         // the last parameter of stokesFromIndex just defines the frame (i.e. linear, circular) and can be
         // any product from the chosen frame. We may want to specify the frame via the parset eventually.
         chunk->stokes()(polIndex) = scimath::PolConverter::stokesFromIndex(polIndex, casa::Stokes::XX);
    }

    // Add the scan index
    chunk->scan() = itsScanManager.scanIndex();

    chunk->targetName() = metadata.targetName();
    chunk->directionFrame() = metadata.phaseDirection().getRef();

    // Determine and add the spectral channel width
    chunk->channelWidth() = corrMode.chanWidth().getValue("Hz");

    // Build frequencies vector
    // Frequency vector is not of length nRows, but instead nChannels
    chunk->frequency() = itsChannelManager.localFrequencies(itsId,
            metadata.centreFreq().getValue("Hz"),
            corrMode.chanWidth().getValue("Hz"),
            corrMode.nChan());

    casa::uInt row = 0;
    const casa::MDirection phaseDir = metadata.phaseDirection();
    for (casa::uInt beam = 0; beam < itsNBeams; ++beam) {
        for (casa::uInt ant1 = 0; ant1 < nAntenna; ++ant1) {
            for (casa::uInt ant2 = ant1; ant2 < nAntenna; ++ant2) {
                ASKAPCHECK(row <= nRow, "Row index (" << row <<
                        ") should not exceed nRow (" << nRow <<")");

                chunk->antenna1()(row) = ant1;
                chunk->antenna2()(row) = ant2;
                chunk->beam1()(row) = beam;
                chunk->beam2()(row) = beam;
                chunk->beam1PA()(row) = 0;
                chunk->beam2PA()(row) = 0;
                chunk->phaseCentre1()(row) = phaseDir.getAngle();
                chunk->phaseCentre2()(row) = phaseDir.getAngle();
                chunk->uvw()(row) = 0.0;

                row++;
            }
        }
    }

    // Populate the per-antenna vectors
    for (casa::uInt i = 0; i < nAntenna; ++i) {
        const string antName = itsConfig.antennas()[i].name();
        const TosMetadataAntenna mdant = metadata.antenna(antName);
        chunk->targetPointingCentre()[i] = metadata.targetDirection();
        chunk->actualPointingCentre()[i] = mdant.actualRaDec();
        chunk->actualPolAngle()[i] = mdant.actualPolAngle();
    }

    return chunk;
}

bool MergedSource::addVis(VisChunk::ShPtr chunk, const VisDatagram& vis,
        const TosMetadata& metadata,
        std::set<DatagramIdentity>& receivedDatagrams)
{
    // 0) Map from baseline to antenna pair and stokes type
    if (itsBaselineMap.idToAntenna1(vis.baselineid) == -1 ||
        itsBaselineMap.idToAntenna2(vis.baselineid) == -1 ||
        itsBaselineMap.idToStokes(vis.baselineid) == casa::Stokes::Undefined) {
            ASKAPLOG_WARN_STR(logger, "Baseline id: " << vis.baselineid
                    << " has no valid mapping to antenna pair and stokes");
        return false;
    }
    const casa::uInt antenna1 = itsBaselineMap.idToAntenna1(vis.baselineid);
    const casa::uInt antenna2 = itsBaselineMap.idToAntenna2(vis.baselineid);
    const casa::Int beamid = itsBeamIDMap(vis.beamid);
    if (beamid < 0) {
        // this beam ID is intentionally unmapped
        return false;
    }
    ASKAPCHECK(beamid < static_cast<casa::Int>(itsNBeams),
        "Received beam id vis.beamid=" << vis.beamid << " mapped to beamid=" << beamid
        << " which is outside the beam index range, itsNBeams=" << itsNBeams);

    // 1) Map from baseline to stokes type and find the  position on the stokes
    // axis of the cube to insert the data into
    const casa::Stokes::StokesTypes stokestype = itsBaselineMap.idToStokes(vis.baselineid);
    // we could use scimath::PolConverter::getIndex here, but the following code allows more checks
    int polidx = -1;
    for (size_t i = 0; i < chunk->stokes().size(); ++i) {
        if (chunk->stokes()(i) == stokestype) {
            polidx = i;
        }
    }
    if (polidx < 0) {
            ASKAPLOG_WARN_STR(logger, "Stokes type " << casa::Stokes::name(stokestype)
                    << " is not configured for storage");
        return false;
    }


    // 2) Check the indexes in the VisDatagram are valid
    const casa::uInt nAntenna = itsConfig.antennas().size();
    ASKAPCHECK(antenna1 < nAntenna, "Antenna 1 index is invalid");
    ASKAPCHECK(antenna2 < nAntenna, "Antenna 2 index is invalid");
    ASKAPCHECK(static_cast<casa::uInt>(beamid) < itsNBeams,
        "Beam index " << beamid << " is invalid");
    ASKAPCHECK(polidx < 4, "Only 4 polarisation products are supported");

    // 3) Detect duplicate datagrams
    const DatagramIdentity identity(vis.baselineid, vis.slice, vis.beamid);
    if (receivedDatagrams.find(identity) != receivedDatagrams.end()) {
        ASKAPLOG_WARN_STR(logger, "Duplicate VisDatagram - BaselineID: " << vis.baselineid
                << ", Slice: " << vis.slice << ", Beam: " << vis.beamid);
        return false;
    }
    receivedDatagrams.insert(identity);

    // 4) Find the row for the given beam and baseline
    // TODO: This is slow, need to develop an indexing method
    casa::uInt row = 0;
    casa::uInt idx = 0;
    for (casa::uInt beam = 0; beam < itsNBeams; ++beam) {
        for (casa::uInt ant1 = 0; ant1 < nAntenna; ++ant1) {
            for (casa::uInt ant2 = ant1; ant2 < nAntenna; ++ant2) {
                if (ant1 == antenna1 &&
                        ant2 == antenna2 &&
                        beam == static_cast<casa::uInt>(beamid)) {
                    row = idx;
                    break;
                }
                idx++;
            }
        }
    }
    static const std::string errorMsg = "Indexing failed to find row";
    ASKAPCHECK(chunk->antenna1()(row) == antenna1, errorMsg);
    ASKAPCHECK(chunk->antenna2()(row) == antenna2, errorMsg);
    ASKAPCHECK(chunk->beam1()(row) == static_cast<casa::uInt>(beamid), errorMsg);
    ASKAPCHECK(chunk->beam2()(row) == static_cast<casa::uInt>(beamid), errorMsg);

    // 5) Does the TOS say this antenna should be flagged?
    const string antName1 = itsConfig.antennas()[antenna1].name();
    const string antName2 = itsConfig.antennas()[antenna2].name();
    const TosMetadataAntenna mdant1 = metadata.antenna(antName1);
    const TosMetadataAntenna mdant2 = metadata.antenna(antName2);
    const bool flagged = metadata.flagged()
        || mdant1.flagged() || !mdant1.onSource()
        || mdant2.flagged() || !mdant2.onSource();

    // 6) Determine the channel offset and add the visibilities
    ASKAPCHECK(vis.slice < 16, "Slice index is invalid");
    const casa::uInt chanOffset = (vis.slice) * N_CHANNELS_PER_SLICE;
    for (casa::uInt chan = 0; chan < N_CHANNELS_PER_SLICE; ++chan) {
        ASKAPCHECK((chanOffset + chan) <= chunk->nChannel(), "Channel index overflow");
        const casa::Complex sample(vis.vis[chan].real, vis.vis[chan].imag);
        chunk->visibility()(row, chanOffset + chan, polidx) = sample;

        // Unflag the sample if TOS metadata indicates it is ok
        if (!flagged) chunk->flag()(row, chanOffset + chan, polidx) = false;

        if (antenna1 == antenna2) {
            // for auto-correlations we duplicate cross-pols as index 2 should always be missing
            ASKAPDEBUGASSERT(polidx != 2);
            if (polidx == 1) {
                chunk->visibility()(row, chanOffset + chan, 2) = conj(sample);
                if (!flagged) chunk->flag()(row, chanOffset + chan, 2) = false;
            }
        }
    }
    return true;
}

void MergedSource::signalHandler(const boost::system::error_code& error,
                                     int signalNumber)
{
    if (signalNumber == SIGTERM || signalNumber == SIGINT || signalNumber == SIGUSR1) {
        itsInterrupted = true;
    }
}

void MergedSource::parseBeamMap(const LOFAR::ParameterSet& params)
{
    const std::string beamidmap = params.getString("beammap","");
    if (beamidmap != "") {
        ASKAPLOG_INFO_STR(logger, "Beam indices will be mapped according to [" <<beamidmap << "]");
        itsBeamIDMap.add(beamidmap);
    }

    // The below implies the beams being received must be a subset (though not
    // necessarily a proper subset) of the beams in the config
    itsNBeams = itsConfig.feed().nFeeds();
}

void MergedSource::checkInterruptSignal()
{
    itsIOService.poll();
    if (itsInterrupted) {
        throw InterruptedException();
    }
}
