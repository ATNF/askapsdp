/// @file NoMetadataSource.cc
///
/// @copyright (c) 2013 CSIRO
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
#include "NoMetadataSource.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "boost/scoped_ptr.hpp"
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "Common/ParameterSet.h"
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
#include "ingestpipeline/sourcetask/ChannelManager.h"
#include "ingestpipeline/sourcetask/InterruptedException.h"
#include "configuration/Configuration.h"
#include "configuration/BaselineMap.h"
#include "monitoring/MonitorPoint.h"

ASKAP_LOGGER(logger, ".NoMetadataSource");

using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;

NoMetadataSource::NoMetadataSource(const LOFAR::ParameterSet& params,
                                   const Configuration& config,
                                   IVisSource::ShPtr visSrc,
                                   int numTasks, int id) :
        itsConfig(config),
        itsVisSrc(visSrc),
        itsNumTasks(numTasks), itsId(id),
        itsChannelManager(params),
        itsBaselineMap(config.bmap()),
        itsInterrupted(false),
        itsSignals(itsIOService, SIGINT, SIGTERM, SIGUSR1),
        itsMaxNBeams(params.getUint32("maxbeams",0)),
        itsBeamsToReceive(params.getUint32("beams2receive",0)),
        itsDuplicateDatagrams(0)
{
    // Trigger a dummy frame conversion with casa measures to ensure all caches are setup
    const casa::MVEpoch dummyEpoch(56000.);

    casa::MEpoch::Convert(casa::MEpoch(dummyEpoch, casa::MEpoch::Ref(casa::MEpoch::TAI)),
                          casa::MEpoch::Ref(casa::MEpoch::UTC))();
    ASKAPCHECK(itsConfig.observation().scans().size() == 1,
               "NoMetadataSource supports only a single scan");

    parseBeamMap(params);

    // Send "obs" monitoring data for scan0
    submitObsMonitorPoints();

    // Setup a signal handler to catch SIGINT, SIGTERM and SIGUSR1
    itsSignals.async_wait(boost::bind(&NoMetadataSource::signalHandler, this, _1, _2));
}

NoMetadataSource::~NoMetadataSource()
{
    itsSignals.cancel();

    // Invalidate the monitoring data points - by sending null to MoniCA
    submitNullMonitorPoints();
}

VisChunk::ShPtr NoMetadataSource::next(void)
{
    // Get the next VisDatagram if there isn't already one in the buffer
    while (!itsVis) {
        itsVis = itsVisSrc->next(10000000); // 1 second timeout

        itsIOService.poll();
        if (itsInterrupted) {
            throw InterruptedException();
        }
    }

    // This is the BAT timestamp for the current integration being processed
    const casa::uLong currentTimestamp = itsVis->timestamp;

    // Now the streams are synced, start building a VisChunk
    VisChunk::ShPtr chunk = createVisChunk(currentTimestamp);

    // Determine how many VisDatagrams are expected for a single integration
    const Scan scanInfo = itsConfig.observation().scans().at(0);
    const casa::uInt nAntenna = itsConfig.antennas().size();
    const casa::uInt nChannels = itsChannelManager.localNChannels(itsId);
    ASKAPCHECK(nChannels % N_CHANNELS_PER_SLICE == 0,
               "Number of channels must be divisible by N_CHANNELS_PER_SLICE");
    const casa::uInt datagramsExpected = itsBaselineMap.size() * itsBeamsToReceive * (nChannels / N_CHANNELS_PER_SLICE);
    const casa::uInt timeout = scanInfo.interval() * 2;

    // Read VisDatagrams and add them to the VisChunk. If itsVisSrc->next()
    // returns a null pointer this indicates the timeout has been reached.
    // In this case assume no more VisDatagrams for this integration will
    // be recieved and move on
    casa::uInt datagramCount = 0;
    casa::uInt datagramsIgnored = 0;
    while (itsVis && currentTimestamp >= itsVis->timestamp) {
        itsIOService.poll();
        if (itsInterrupted) {
            throw InterruptedException();
        }

        if (currentTimestamp > itsVis->timestamp) {
            // If the VisDatagram is from a prior integration then discard it
            ASKAPLOG_WARN_STR(logger, "Received VisDatagram from past integration");
            itsVis = itsVisSrc->next(timeout);
            continue;
        }

        datagramCount++;
        if (addVis(chunk, *itsVis, nAntenna)) {
            ++datagramsIgnored;
        }
        itsVis = itsVisSrc->next(timeout);
        if (datagramCount == datagramsExpected) {
            // This integration is finished
            break;
        }
    }

    ASKAPLOG_DEBUG_STR(logger, "VisChunk built with " << datagramCount <<
                       " of expected " << datagramsExpected << " visibility datagrams");
    ASKAPLOG_DEBUG_STR(logger, "     - ignored " << datagramsIgnored
            << " successfully received datagrams");
    if (itsDuplicateDatagrams > 0) {
        ASKAPLOG_WARN_STR(logger, "     - " << itsDuplicateDatagrams
                << " duplicate datagrams received");
        itsDuplicateDatagrams = 0;
    }

    // Submit monitoring data
    MonitorPoint<int32_t> packetsLostCount("PacketsLostCount");
    packetsLostCount.update(datagramsExpected - datagramCount);
    if (datagramsExpected != 0) {
        MonitorPoint<float> packetsLostPercent("PacketsLostPercent");
        packetsLostPercent.update((datagramsExpected - datagramCount)
                / static_cast<float>(datagramsExpected) * 100.);
    }

    return chunk;
}

VisChunk::ShPtr NoMetadataSource::createVisChunk(const casa::uLong timestamp)
{
    /*
    ASKAPLOG_DEBUG_STR(logger, "received chunk at bat="<<timestamp<<" == 0x"<<std::hex<<timestamp);
    ASKAPLOG_DEBUG_STR(logger, "diff: "<<std::setprecision(9)<<double(timestamp-0x11662f89a887f0)/4976640.<<" cycles ");
    */
    const Scan scanInfo = itsConfig.observation().scans().at(0);
    const casa::uInt nAntenna = itsConfig.antennas().size();
    ASKAPCHECK(nAntenna > 0, "Must have at least one antenna defined");
    const casa::uInt nChannels = itsChannelManager.localNChannels(itsId);
    const casa::uInt nPol = scanInfo.stokes().size();
    const casa::uInt nBaselines = nAntenna * (nAntenna + 1) / 2;
    const casa::uInt nRow = nBaselines * itsMaxNBeams;
    const casa::uInt period = scanInfo.interval(); // in microseconds

    VisChunk::ShPtr chunk(new VisChunk(nRow, nChannels, nPol));

    // Convert the time from integration start in microseconds to an
    // integration mid-point in seconds
    const uint64_t midpointBAT = static_cast<uint64_t>(timestamp + (period / 2ull));
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
    chunk->scan() = 0;

    // Determine and add the spectral channel width
    chunk->channelWidth() = scanInfo.chanWidth().getValue("Hz");

    casa::uInt row = 0;
    for (casa::uInt beam = 0; beam < itsMaxNBeams; ++beam) {
        for (casa::uInt ant1 = 0; ant1 < nAntenna; ++ant1) {
            for (casa::uInt ant2 = ant1; ant2 < nAntenna; ++ant2) {
                ASKAPCHECK(row < nRow, "Row index (" << row <<
                           ") should be less than nRow (" << nRow << ")");

                // TODO!!
                // The handling of pointing directions below is not handled per beam.
                // It just takes the field centre direction from the parset and uses
                // that for all beam pointing directions.
                chunk->directionFrame() = scanInfo.fieldDirection().getRef();

                chunk->antenna1()(row) = ant1;
                chunk->antenna2()(row) = ant2;
                chunk->beam1()(row) = beam;
                chunk->beam2()(row) = beam;
                chunk->beam1PA()(row) = 0;
                chunk->beam2PA()(row) = 0;
                chunk->pointingDir1()(row) = scanInfo.fieldDirection().getAngle();
                chunk->pointingDir2()(row) = scanInfo.fieldDirection().getAngle();
                chunk->dishPointing1()(row) = scanInfo.fieldDirection().getAngle();
                chunk->dishPointing2()(row) = scanInfo.fieldDirection().getAngle();
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

bool NoMetadataSource::addVis(VisChunk::ShPtr chunk, const VisDatagram& vis,
                              const casa::uInt nAntenna)
{
    // 0) Map from baseline to antenna pair and stokes type
    if (itsBaselineMap.idToAntenna1(vis.baselineid) == -1 ||
        itsBaselineMap.idToAntenna2(vis.baselineid) == -1 ||
        itsBaselineMap.idToStokes(vis.baselineid) == casa::Stokes::Undefined) {
            ASKAPLOG_WARN_STR(logger, "Baseline id: " << vis.baselineid
                    << " has no valid mapping to antenna pair and stokes");
        return true;
    }

    const casa::uInt antenna1 = itsBaselineMap.idToAntenna1(vis.baselineid);
    const casa::uInt antenna2 = itsBaselineMap.idToAntenna2(vis.baselineid);
    const casa::Int beamid = itsBeamIDMap(vis.beamid);
    if (beamid < 0) {
        // this beam ID is intentionally unmapped
        return true;
    }
    ASKAPCHECK(beamid < static_cast<casa::Int>(itsMaxNBeams), 
               "Received beam id vis.beamid="<<vis.beamid<<" mapped to beamid="<<beamid<<
               " which is outside the beam index range, itsMaxNBeams="<<itsMaxNBeams);

    // 1) Map from baseline to stokes type and find the  position on the stokes
    // axis of the cube to insert the data into
    const casa::Stokes::StokesTypes stokestype = itsBaselineMap.idToStokes(vis.baselineid);
    // We could use scimath::PolConverter::getIndex here, but the following code allows more checks
    int polidx = -1;
    for (size_t i = 0; i < chunk->stokes().size(); ++i) {
        if (chunk->stokes()(i) == stokestype) {
            polidx = i;
        }
    }
    if (polidx < 0) {
        ASKAPLOG_WARN_STR(logger, "Stokes type " << casa::Stokes::name(stokestype)
                              << " is not configured for storage");
        return true;
    }


    // 2) Check the indexes in the VisDatagram are valid
    ASKAPCHECK(antenna1 < nAntenna, "Antenna 1 index is invalid");
    ASKAPCHECK(antenna2 < nAntenna, "Antenna 2 index is invalid");
    ASKAPCHECK(static_cast<casa::uInt>(beamid) < itsMaxNBeams, "Beam index " << beamid << " is invalid");
    ASKAPCHECK(polidx < 4, "Only 4 polarisation products are supported");

    // 3) Find the row for the given beam and baseline
    // TODO: This is slow, need to develop an indexing method
    casa::uInt row = 0;
    casa::uInt idx = 0;
    for (casa::uInt beam = 0; beam < itsMaxNBeams; ++beam) {
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

    const std::string errorMsg = "Indexing failed to find row";
    ASKAPCHECK(chunk->antenna1()(row) == antenna1, errorMsg);
    ASKAPCHECK(chunk->antenna2()(row) == antenna2, errorMsg);
    ASKAPCHECK(chunk->beam1()(row) == static_cast<casa::uInt>(beamid), errorMsg);
    ASKAPCHECK(chunk->beam2()(row) == static_cast<casa::uInt>(beamid), errorMsg);

    // 4) Determine the channel offset and add the visibilities
    ASKAPCHECK(vis.slice < 16, "Slice index is invalid");
    const casa::uInt chanOffset = (vis.slice) * N_CHANNELS_PER_SLICE;
    for (casa::uInt chan = 0; chan < N_CHANNELS_PER_SLICE; ++chan) {
        // If the sample is already "unflagged" it means we have received it,
        // and this VisChnk is a duplicate
        if (chunk->flag()(row, chanOffset + chan, polidx) == false) {
            ++itsDuplicateDatagrams;
            return true;
        }

        casa::Complex sample(vis.vis[chan].real, vis.vis[chan].imag);
        ASKAPCHECK((chanOffset + chan) <= chunk->nChannel(), "Channel index overflow");

        chunk->visibility()(row, chanOffset + chan, polidx) = sample;

        // Unflag the sample
        chunk->flag()(row, chanOffset + chan, polidx) = false;

        if (antenna1 == antenna2) {
            // For auto-correlations we duplicate cross-pols as index 2 should always be missing
            ASKAPDEBUGASSERT(polidx != 2);

            if (polidx == 1) {
                chunk->visibility()(row, chanOffset + chan, 2) = conj(sample);
                // Unflag the sample
                chunk->flag()(row, chanOffset + chan, 2) = false;
            }
        }
    }
    return false;
}

void NoMetadataSource::signalHandler(const boost::system::error_code& error,
                                     int signalNumber)
{
    if (signalNumber == SIGTERM || signalNumber == SIGINT || signalNumber == SIGUSR1) {
        itsInterrupted = true;
    }
}

void NoMetadataSource::parseBeamMap(const LOFAR::ParameterSet& params)
{                             
    const std::string beamidmap = params.getString("beammap","");
    if (beamidmap != "") {    
        ASKAPLOG_INFO_STR(logger, "Beam indices will be mapped according to <"<<beamidmap<<">");
        itsBeamIDMap.add(beamidmap);
    }   
    const casa::uInt nBeamsInConfig = itsConfig.antennas().at(0).feeds().nFeeds();
    if (itsMaxNBeams == 0) {
        for (int beam = 0; beam < static_cast<int>(nBeamsInConfig) + 1; ++beam) {
             const int processedBeamIndex = itsBeamIDMap(beam);
             if (processedBeamIndex > static_cast<int>(itsMaxNBeams)) {
                 // negative values are automatically excluded by this condition
                 itsMaxNBeams = static_cast<casa::uInt>(processedBeamIndex);
             }
        }
        ++itsMaxNBeams;
    }   
    if (itsBeamsToReceive == 0) {
        itsBeamsToReceive = nBeamsInConfig;
    }        
    ASKAPLOG_INFO_STR(logger, "Number of beams: " << nBeamsInConfig << " (defined in configuration), "
            << itsBeamsToReceive << " (to be received), " << itsMaxNBeams << " (to be written into MS)");
    ASKAPDEBUGASSERT(itsMaxNBeams > 0);
    ASKAPDEBUGASSERT(itsBeamsToReceive > 0);
}

void NoMetadataSource::submitObsMonitorPoints() const
{
    submitPoint<int32_t>("obs.nScans", 1);
    submitPoint<int32_t>("obs.ScanId", 0);
    const Observation obs = itsConfig.observation();
    const Scan s = obs.scans()[0];
    submitPoint<string>("obs.FieldName", s.name());
    submitPoint<string>("obs.dir1", askap::printLat(s.fieldDirection()));
    submitPoint<string>("obs.dir2", askap::printLon(s.fieldDirection()));
    submitPoint<string>("obs.CoordSys", casa::MDirection::showType(s.fieldDirection().type()));
    submitPoint<int32_t>("obs.Interval", s.interval() / 1000);
    submitPoint<float>("obs.StartFreq", s.startFreq().getValue("MHz"));
    submitPoint<int32_t>("obs.nChan", s.nChan());
    submitPoint<float>("obs.ChanWidth", s.chanWidth().getValue("kHz"));
}

void NoMetadataSource::submitNullMonitorPoints() const
{
    submitPointNull("obs.nScans");
    submitPointNull("obs.ScanId");
    submitPointNull("obs.FieldName");
    submitPointNull("obs.dir1");
    submitPointNull("obs.dir2");
    submitPointNull("obs.CoordSys");
    submitPointNull("obs.Interval");
    submitPointNull("obs.StartFreq");
    submitPointNull("obs.nChan");
    submitPointNull("obs.ChanWidth");

    submitPointNull("PacketsLostCount");
    submitPointNull("PacketsLostPercent");
}

void NoMetadataSource::submitPointNull(const std::string& key) const
{
    MonitorPoint<int32_t> point(key);
    point.updateNull();
}
