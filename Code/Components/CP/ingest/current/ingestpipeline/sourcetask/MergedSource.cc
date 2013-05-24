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
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"
#include "cpcommon/TosMetadata.h"
#include "cpcommon/VisDatagram.h"
#include "utils/PolConverter.h"
#include "casa/Quanta/MVEpoch.h"
#include "measures/Measures.h"
#include "measures/Measures/MeasFrame.h"
#include "measures/Measures/MCEpoch.h"

// Local package includes
#include "ingestpipeline/sourcetask/IVisSource.h"
#include "ingestpipeline/sourcetask/IMetadataSource.h"
#include "ingestpipeline/sourcetask/ScanManager.h"
#include "ingestpipeline/sourcetask/ChannelManager.h"
#include "configuration/Configuration.h"
#include "configuration/BaselineMap.h"
#include "monitoring/MonitorPoint.h"

ASKAP_LOGGER(logger, ".MergedSource");

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
     itsBaselineMap(config.bmap())
{
  // trigger a dummy frame conversion with casa measures to ensure all chaches are setup
  const casa::MVEpoch dummyEpoch(56000.);

  casa::MEpoch::Convert(casa::MEpoch(dummyEpoch, casa::MEpoch::Ref(casa::MEpoch::TAI)),
                             casa::MEpoch::Ref(casa::MEpoch::UTC))();
}

MergedSource::~MergedSource()
{
}

VisChunk::ShPtr MergedSource::next(void)
{
    // Get the next TosMetadata
    do {
        itsMetadata = itsMetadataSrc->next();

        if (!itsMetadata->antenna(0).scanActive()) {
            ASKAPLOG_DEBUG_STR(logger, "Received telescope metadata with scan_active false");
        }

        // Update the Scan Manager
        itsScanManager.update(itsMetadata->antenna(0).scanActive(),
                itsMetadata->antenna(0).scanId());

        // Check if the TOS/TOM has indicated the observation is complete
        if (itsScanManager.observationComplete()) {
            ASKAPLOG_INFO_STR(logger, "End-of-observation condition met");
            return VisChunk::ShPtr();
        }
    } while (!itsMetadata->antenna(0).scanActive());

    // Get the next VisDatagram if there isn't already one in the buffer
    if (!itsVis) {
        itsVis = itsVisSrc->next();
    }

    // Find data with matching timestamps
    while (itsMetadata->time() != itsVis->timestamp) {

        // If the VisDatagram timestamps are in the past (with respect to the
        // TosMetadata) then read VisDatagrams until they catch up
        while (itsMetadata->time() > itsVis->timestamp) {
            ASKAPLOG_DEBUG_STR(logger, "Reading an extra VisDatagram to catch up");
            itsVis = itsVisSrc->next();
        }

        // But if the timestamp in the VisDatagram is in the future (with
        // respect to the TosMetadata) then it is time to fetch new TosMetadata
        if (itsMetadata->time() < itsVis->timestamp) {
            ASKAPLOG_DEBUG_STR(logger, "Reading an extra TosMetadata to catch up");
            itsMetadata = itsMetadataSrc->next();
        }
    }

    // Now the streams are synced, start building a VisChunk
    VisChunk::ShPtr chunk = createVisChunk(*itsMetadata);

    // Determine how many VisDatagrams are expected for a single integration
    const Scan scanInfo = itsConfig.observation().scans().at(itsScanManager.scanIndex());
    const casa::uInt nAntenna = itsMetadata->nAntenna();
    const casa::uInt nChannels = itsChannelManager.localNChannels(itsId);
    const casa::uInt nBeams = itsMetadata->nBeams();
    ASKAPCHECK(nChannels % N_CHANNELS_PER_SLICE == 0,
            "Number of channels must be divisible by N_CHANNELS_PER_SLICE");
    const casa::uInt datagramsExpected =itsBaselineMap.size() * nBeams * (nChannels / N_CHANNELS_PER_SLICE);
    const casa::uInt timeout = scanInfo.interval() * 2;

    // Read VisDatagrams and add them to the VisChunk. If itsVisSrc->next()
    // returns a null pointer this indicates the timeout has been reached.
    // In this case assume no more VisDatagrams for this integration will
    // be recieved and move on
    casa::uInt datagramCount = 0; 
    while (itsVis && itsMetadata->time() >= itsVis->timestamp) {
        if (itsMetadata->time() > itsVis->timestamp) {
            // If the VisDatagram is from a prior integration then discard it
            ASKAPLOG_WARN_STR(logger, "Received VisDatagram from past integration");
            itsVis = itsVisSrc->next(timeout);
            continue;
        }

        datagramCount++;
        addVis(chunk, *itsVis, nAntenna, nBeams);
        itsVis = itsVisSrc->next(timeout);
        if (datagramCount == datagramsExpected) {
            // This integration is finished
            break;
        }
    }

    ASKAPLOG_DEBUG_STR(logger, "VisChunk built with " << datagramCount <<
            " of expected " << datagramsExpected << " visibility datagrams");

    // Submit monitoring data
    MonitorPoint<float> datagramLoss("datagram_loss");
    datagramLoss.update((static_cast<float>(datagramsExpected) - static_cast<float>(datagramCount))
            / static_cast<float>(datagramsExpected));

    // Apply any flagging specified in the TOS metadata
    doFlagging(chunk, *itsMetadata);

    itsMetadata.reset();
    return chunk;
}

VisChunk::ShPtr MergedSource::createVisChunk(const TosMetadata& metadata)
{
    const Scan scanInfo = itsConfig.observation().scans().at(itsScanManager.scanIndex());
    const casa::uInt nAntenna = metadata.nAntenna();
    const casa::uInt nChannels = itsChannelManager.localNChannels(itsId);
    const casa::uInt nBeams = metadata.nBeams();
    const casa::uInt nPol = metadata.nPol();
    const casa::uInt nBaselines = nAntenna * (nAntenna + 1) / 2;
    const casa::uInt nRow = nBaselines * nBeams;
    const casa::uInt period = scanInfo.interval(); // in microseconds

    VisChunk::ShPtr chunk(new VisChunk(nRow, nChannels, nPol));

    // Convert the time from integration start in microseconds to an
    // integration mid-point in seconds
    // we have to work with 64-bit integers explicitly and ensure the required accuracy
    // is achieved when converting to MVEpoch
    const uint64_t midpointBAT = static_cast<uint64_t>(metadata.time()) + (period / 2ull);
    const uint64_t microsecondsPerDay = 86400000000ull;
    const casa::MVEpoch BATEpoch(static_cast<casa::Double>(midpointBAT / microsecondsPerDay), 
                                 static_cast<casa::Double>(midpointBAT % microsecondsPerDay) / 
                                 static_cast<casa::Double>(microsecondsPerDay));
    chunk->time() = casa::MEpoch::Convert(casa::MEpoch(BATEpoch, casa::MEpoch::Ref(casa::MEpoch::TAI)),
                             casa::MEpoch::Ref(casa::MEpoch::UTC))().getValue();
  
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

    // Determine and add the spectral channel width
    chunk->channelWidth() = scanInfo.chanWidth().getValue("Hz");

    casa::uInt row = 0;
    for (casa::uInt beam = 0; beam < nBeams; ++beam) {
        for (casa::uInt ant1 = 0; ant1 < nAntenna; ++ant1) {
            const TosMetadataAntenna& mdAnt1 = metadata.antenna(ant1);
            for (casa::uInt ant2 = ant1; ant2 < nAntenna; ++ant2) {
                ASKAPCHECK(row <= nRow, "Row index (" << row <<
                        ") should not exceed nRow (" << nRow <<")");
                const TosMetadataAntenna& mdAnt2 = metadata.antenna(ant2);

                // Set thedirection reference if it is not already set, otherwise
                // check to ensure all MDirection instances have the same reference
                // frame.
                if (chunk->directionFrame().getType() == casa::MDirection::DEFAULT) {
                    chunk->directionFrame() = mdAnt1.phaseTrackingCentre(beam).getRef();
                } else {
                    const casa::uInt type = chunk->directionFrame().getType();
                    const std::string errMsg = "Direction reference inconsistant";
                    ASKAPCHECK(type == mdAnt1.phaseTrackingCentre(beam).getRef().getType(), errMsg);
                    ASKAPCHECK(type == mdAnt2.phaseTrackingCentre(beam).getRef().getType(), errMsg);
                    ASKAPCHECK(type == mdAnt1.targetRaDec().getRef().getType(), errMsg);
                    ASKAPCHECK(type == mdAnt2.targetRaDec().getRef().getType(), errMsg);
                }
                
                chunk->antenna1()(row) = ant1;
                chunk->antenna2()(row) = ant2;
                chunk->beam1()(row) = beam;
                chunk->beam2()(row) = beam;
                chunk->beam1PA()(row) = mdAnt1.polarisationOffset();
                chunk->beam2PA()(row) = mdAnt2.polarisationOffset();
                chunk->pointingDir1()(row) = mdAnt1.phaseTrackingCentre(beam).getAngle();
                chunk->pointingDir2()(row) = mdAnt2.phaseTrackingCentre(beam).getAngle();
                chunk->dishPointing1()(row) = mdAnt1.targetRaDec().getAngle();
                chunk->dishPointing2()(row) = mdAnt2.targetRaDec().getAngle();
                chunk->uvw()(row) = 0.0;
                
                // Frequency vector is not of length nRows, but instead nChannels
                chunk->frequency() = itsChannelManager.localFrequencies(itsId,
                        scanInfo.startFreq().getValue("Hz"),
                        scanInfo.chanWidth().getValue("Hz"));

                row++;
            }
        }
    }

    return chunk;
}

void MergedSource::addVis(VisChunk::ShPtr chunk, const VisDatagram& vis,
        const casa::uInt nAntenna, const casa::uInt nBeams)
{
    // 0) Map from baseline to antenna pair and stokes type
    if (itsBaselineMap.idToAntenna1(vis.baselineid) == -1 ||
            itsBaselineMap.idToAntenna2(vis.baselineid) == -1) {
            ASKAPLOG_WARN_STR(logger, "Baseline id: " << vis.baselineid
                    << " has no valid mapping to antenna pair and stokes");
        return;
    }
    const casa::uInt antenna1 = itsBaselineMap.idToAntenna1(vis.baselineid);
    const casa::uInt antenna2 = itsBaselineMap.idToAntenna2(vis.baselineid);
    ASKAPCHECK(vis.beamid > 0, "Expected one-based indexing for beam ID");
    const casa::uInt beamid = vis.beamid - 1;

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
        return;
    }


    // 2) Check the indexes in the VisDatagram are valid
    ASKAPCHECK(antenna1 < nAntenna, "Antenna 1 index is invalid");
    ASKAPCHECK(antenna2 < nAntenna, "Antenna 2 index is invalid");
    ASKAPCHECK(beamid < nBeams, "Beam index " << beamid << " is invalid");
    ASKAPCHECK(polidx < 4, "Only 4 polarisation products are supported");

    // 3) Find the row for the given beam and baseline
    // TODO: This is slow, need to develop an indexing method
    casa::uInt row = 0;
    casa::uInt idx = 0;
    for (casa::uInt beam = 0; beam < nBeams; ++beam) {
        for (casa::uInt ant1 = 0; ant1 < nAntenna; ++ant1) {
            for (casa::uInt ant2 = ant1; ant2 < nAntenna; ++ant2) {
                if (ant1 == antenna1 &&
                        ant2 == antenna2 &&
                        beam == beamid) {
                    row = idx;
                    break;
                }
                idx++;
            }
        }
    }
    const std::string errorMsg = "Indexing failed to find row";
    ASKAPCHECK(chunk->antenna1()(row) == antenna1, errorMsg);
    ASKAPCHECK(chunk->antenna2()(row) == antenna2, errorMsg);
    ASKAPCHECK(chunk->beam1()(row) == beamid, errorMsg);
    ASKAPCHECK(chunk->beam2()(row) == beamid, errorMsg);

    // 4) Determine the channel offset and add the visibilities
    ASKAPCHECK(vis.slice < 16, "Slice index is invalid");
    const casa::uInt chanOffset = (vis.slice) * N_CHANNELS_PER_SLICE;
    for (casa::uInt chan = 0; chan < N_CHANNELS_PER_SLICE; ++chan) {
        casa::Complex sample(vis.vis[chan].real, vis.vis[chan].imag);
        ASKAPCHECK((chanOffset + chan) <= chunk->nChannel(), "Channel index overflow");

        chunk->visibility()(row, chanOffset + chan, polidx) = sample;

        // Unflag the sample
        chunk->flag()(row, chanOffset + chan, polidx) = false;

        if (antenna1 == antenna2) {
            // for auto-correlations we duplicate cross-pols as index 2 should always be missing
            ASKAPDEBUGASSERT(polidx != 2);
            if (polidx == 1) {
                chunk->visibility()(row, chanOffset + chan, 2) = conj(sample);
                // unflag the sample
                chunk->flag()(row, chanOffset + chan, 2) = false;
            }
        }
    }
}

// Flag based on information in the TosMetadata
void MergedSource::doFlagging(VisChunk::ShPtr chunk, const TosMetadata& metadata)
{
    for (unsigned int row = 0; row < chunk->nRow(); ++row) {
        for (unsigned int chan = 0; chan < chunk->nChannel(); ++chan) {
            for (unsigned int pol = 0; pol < chunk->nPol(); ++pol) {
                doFlaggingSample(chunk, metadata, row, chan, pol);
            }
        }
    }
}

void MergedSource::doFlaggingSample(VisChunk::ShPtr chunk,
        const TosMetadata& metadata,
        const unsigned int row,
        const unsigned int chan,
        const unsigned int pol)
{
    // Don't bother if the sample is already flagged
    if (chunk->flag()(row, chan, pol)) {
        return;
    }

    const unsigned int ant1 = chunk->antenna1()(row);
    const unsigned int ant2 = chunk->antenna2()(row);
    const TosMetadataAntenna& mdAnt1 = metadata.antenna(ant1);
    const TosMetadataAntenna& mdAnt2 = metadata.antenna(ant2);

    // Flag the sample if one of the antenna was not on source or had a
    // hardware error
    if ((!mdAnt1.onSource()) || (!mdAnt2.onSource()) ||
            mdAnt1.hwError() || mdAnt2.hwError()) {
        chunk->flag()(row, chan, pol) = true;
        return;
    }
}
