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

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/scoped_ptr.hpp"
#include "cpcommon/TosMetadata.h"
#include "cpcommon/VisDatagram.h"
#include "measures/Measures.h"
#include "casa/Quanta/MVEpoch.h"

// Local package includes
#include "ingestpipeline/IngestUtils.h"
#include "ingestpipeline/sourcetask/IVisSource.h"
#include "ingestpipeline/sourcetask/IMetadataSource.h"

ASKAP_LOGGER(logger, ".MergedSource");

using namespace askap;
using namespace askap::cp;

MergedSource::MergedSource(IMetadataSource::ShPtr metadataSrc, IVisSource::ShPtr visSrc) :
    itsMetadataSrc(metadataSrc), itsVisSrc(visSrc)
{
}

MergedSource::~MergedSource()
{
}

VisChunk::ShPtr MergedSource::next(void)
{
    // Get the next TosMetadata
    itsMetadata = itsMetadataSrc->next();

    // Get the next VisDatagram if there isn't already one in the buffer
    if (!itsVis) {
        itsVis = itsVisSrc->next();
    }

    // Find data with matching timestamps
    while (itsMetadata->time() != itsVis->timestamp) {

        // If the VisDatagram timestamps are in the past (with respect to the
        // TosMetadata) then read VisDatagrams until they catch up
        while (itsMetadata->time() > itsVis->timestamp) {
            itsVis = itsVisSrc->next();
        }

        // But if the timestamp in the VisDatagram is in the future (with
        // respect to the TosMetadata) then it is time to fetch new TosMetadata
        if (itsMetadata->time() < itsVis->timestamp) {
            itsMetadata = itsMetadataSrc->next();
        }
    }

    // Now the streams are synced, start building a VisChunk
    VisChunk::ShPtr chunk = createVisChunk(*itsMetadata);

    // Determine how many VisDatagrams are expected for a single integration
    const casa::uInt nAntenna = itsMetadata->nAntenna();
    const casa::uInt nCoarseChannels = itsMetadata->nCoarseChannels();
    const casa::uInt nBeams = itsMetadata->nBeams();
    const casa::uInt nBaselines = nAntenna * (nAntenna + 1) / 2;
    const casa::uInt datagramsExpected = nBaselines * nCoarseChannels * nBeams;
    const casa::uInt timeout = itsMetadata->period() * 2;

    // Read VisDatagrams and add them to the VisChunk. If itsVisSrc->next()
    // returns a null pointer this indicates the timeout has been reached.
    // In this case assume no more VisDatagrams for this integration will
    // be recieved and move on
    casa::uInt datagramCount = 1; 
    while (itsVis && itsMetadata->time() >= itsVis->timestamp) {
        if (itsMetadata->time() > itsVis->timestamp) {
            // If the VisDatagram is from a prior integration then discard it
            ASKAPLOG_WARN_STR(logger, "Received VisDatagram from past integration");
            itsVis = itsVisSrc->next(timeout);
            continue;
        }
        addVis(chunk, *itsVis);
        itsVis = itsVisSrc->next(timeout);
        if (datagramCount == datagramsExpected) {
            // This integration is finished
            break;
        }
        datagramCount++;

    }

    ASKAPLOG_DEBUG_STR(logger, "Integration completed with " << datagramCount <<
            " of expected " << datagramsExpected << " visibility datagrams");

    // Apply any flagging specified in the TOS metadata
    doFlagging(chunk, *itsMetadata);

    itsMetadata.reset();
    return chunk;
}

VisChunk::ShPtr MergedSource::createVisChunk(const TosMetadata& metadata)
{
    const casa::uInt nAntenna = metadata.nAntenna();
    const casa::uInt nCoarseChannels = metadata.nCoarseChannels();
    const casa::uInt nChannels = nCoarseChannels * N_FINE_PER_COARSE;
    const casa::uInt nBeams = metadata.nBeams();
    const casa::uInt nPol = metadata.nPol();
    const casa::uInt nBaselines = nAntenna * (nAntenna + 1) / 2;
    const casa::uInt nRow = nBaselines * nBeams;
    const casa::uInt period = metadata.period();

    VisChunk::ShPtr chunk(new VisChunk(nRow, nChannels, nPol));

    // Convert the time from integration start in microseconds to an
    // integration mid-point in seconds
    const casa::uLong midpoint = metadata.time() + (period / 2ul);
    chunk->time() = casa::MVEpoch(casa::Quantity((static_cast<casa::Double>(midpoint) / 1000000.0), "s"));

    // Convert the interval from microseconds (long) to seconds (double)
    const casa::Double interval = metadata.period() / 1000.0 / 1000.0;
    chunk->interval() = interval;

    // All visibilities get flagged as bad, then as the visibility data 
    // arrives they are unflagged
    chunk->flag() = true;
    chunk->visibility() = 0.0;

    // For now polarisation data is hardcoded.
    // TODO: In the long run this needs to come from some sort of
    // correlator configuration database
    ASKAPCHECK(nPol == 4, "Only supporting 4 polarisation products");
    chunk->stokes()(0) = casa::Stokes::XX;
    chunk->stokes()(1) = casa::Stokes::XY;
    chunk->stokes()(2) = casa::Stokes::YX;
    chunk->stokes()(3) = casa::Stokes::YY;

    casa::uInt row = 0;
    for (casa::uInt beam = 0; beam < nBeams; ++beam) {
        for (casa::uInt ant1 = 0; ant1 < nAntenna; ++ant1) {
            const TosMetadataAntenna& mdAnt1 = metadata.antenna(ant1);
            for (casa::uInt ant2 = ant1; ant2 < nAntenna; ++ant2) {
                ASKAPCHECK(row <= nRow, "Row index (" << row <<
                        ") should not exceed nRow (" << nRow <<")");
                const TosMetadataAntenna& mdAnt2 = metadata.antenna(ant2);

                chunk->antenna1()(row) = ant1;
                chunk->antenna2()(row) = ant2;
                chunk->beam1()(row) = beam;
                chunk->beam2()(row) = beam;
                chunk->beam1PA()(row) = mdAnt1.parallacticAngle();
                chunk->beam2PA()(row) = mdAnt2.parallacticAngle();
                chunk->pointingDir1()(row) = mdAnt1.phaseTrackingCentre(beam, 0);
                chunk->pointingDir2()(row) = mdAnt2.phaseTrackingCentre(beam, 0);
                chunk->dishPointing1()(row) = mdAnt1.dishPointing();
                chunk->dishPointing2()(row) = mdAnt2.dishPointing();
                chunk->frequency()(row) = 0.0;
                chunk->uvw()(row) = 0.0;

                row++;
            }
        }
    }

    return chunk;
}

void MergedSource::addVis(VisChunk::ShPtr chunk, const VisDatagram& vis)
{
    // 1) Find the row for the given beam and baseline
    // TODO: This is slow, need to develop an indexing method
    casa::uInt row = 0;
    while (row < chunk->nRow()) {
        ASKAPCHECK(vis.antenna1 < 36, "Antenna 1 index is invalid");
        ASKAPCHECK(vis.antenna2 < 36, "Antenna 2 index is invalid");
        ASKAPCHECK(vis.beam1 < 36, "Beam 1 index is invalid");
        ASKAPCHECK(vis.beam2 < 36, "Beam 2 index is invalid");
        if ((chunk->antenna1()(row) == vis.antenna1) &&
            (chunk->antenna2()(row) == vis.antenna2) &&
            (chunk->beam1()(row) == vis.beam1) &&
            (chunk->beam2()(row) == vis.beam2)) {
                break;
            }
        ++row;
    }
 
    // 2) Determine the channel offset and add the visibilities
    ASKAPCHECK(vis.coarseChannel < 304, "Coarse channel index is invalid");
    const casa::uInt chanOffset = (vis.coarseChannel) * N_FINE_PER_COARSE;
    for (casa::uInt chan = 0; chan < N_FINE_PER_COARSE; ++chan) {
        for (casa::uInt pol = 0; pol < N_POL; ++pol) {
            const casa::uInt index = pol + ((N_POL) * chan);
            casa::Complex sample(vis.vis[index].real, vis.vis[index].imag);
            ASKAPCHECK((chanOffset + chan) <= chunk->nChannel(), "Channel index overflow");
            chunk->visibility()(row, chanOffset + chan, pol) = sample;

            // Unflag the sample, if nSamples is non-zero. A zero value can
            // indicate the correlator has flagged the sample.
            if (vis.nSamples[index] > 0) {
                chunk->flag()(row, chanOffset + chan, pol) = false;
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

    // Flag if detailed flagging is set in the metadata for this sample.
    // Note flagging in metadata is per coarse channel so if a coarse channel
    // is flagged then the whole 54 fine channels are flagged
    const unsigned int beam1 = chunk->beam1()(row);
    const unsigned int beam2 = chunk->beam2()(row);
    const unsigned int coarseChan = IngestUtils::fineToCoarseChannel(chan);

    if (mdAnt1.flagDetailed(beam1, coarseChan, pol)) {
        chunk->flag()(row, chan, pol) = true;
        return;
    }
    if (mdAnt2.flagDetailed(beam2, coarseChan, pol)) {
        chunk->flag()(row, chan, pol) = true;
        return;
    }

}
