/// @file UVChannelConstDataIterator.cc
/// @brief
///
/// @copyright (c) 2011 CSIRO
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
#include "UVChannelConstDataIterator.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "dataaccess/DataAccessError.h"
#include "dataaccess/DirectionConverter.h"
#include "cpcommon/VisChunk.h"

// Local includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/uvdataaccess/UVChannelConstDataSource.h"
#include "uvchannel/uvdataaccess/UVChannelConstDataAccessor.h"
#include "uvchannel/uvdataaccess/UVChannelDataSelector.h"
#include "uvchannel/uvdataaccess/UVChannelDataConverter.h"
#include "uvchannel/uvdataaccess/UVChannelReceiver.h"

// Using
using namespace casa;
using namespace askap;
using namespace askap::accessors;
using namespace askap::cp::channels;

UVChannelConstDataIterator::UVChannelConstDataIterator(const UVChannelConfig& channelConfig,
        const std::string& channelName,
        const boost::shared_ptr<const UVChannelDataSelector> &sel,
        const boost::shared_ptr<const UVChannelDataConverter> &conv)
        : itsChannelConfig(channelConfig),
        itsChannelName(channelName),
        itsSelector(sel),
        itsConverter(conv)
{
    if (!itsSelector->channelsSelected()) {
        ASKAPTHROW(AskapError, "UVChannelConstDataIterator() no channels selected");
    }

    const casa::uInt nChan = itsSelector->getChannelSelection().first;
    const casa::uInt startChan = itsSelector->getChannelSelection().second;

    itsReceiver.reset(new UVChannelReceiver(channelConfig, channelName, startChan, nChan));
}

void UVChannelConstDataIterator::init()
{
    if (itsConstAccessor.get() == 0) {
        next();
    } else {
        ASKAPTHROW(AskapError, "UVChannelConstDataIterator::init() Can only be initialised once");
    }
}

const IConstDataAccessor& UVChannelConstDataIterator::operator*() const
{
    return *itsConstAccessor;
}

casa::Bool UVChannelConstDataIterator::hasMore() const throw()
{
    const casa::uInt nChan = itsSelector->getChannelSelection().first;
    const casa::uInt startChan = itsSelector->getChannelSelection().second;

    // In order for hasMore() to be true, all channels must have more data.
    for (casa::uInt chan = startChan; chan < startChan + nChan; ++chan) {
        if (!itsReceiver->hasMore(chan)) {
            return false;
        }
    }

    return true;
}

casa::Bool UVChannelConstDataIterator::next()
{
    const casa::uInt nChan = itsSelector->getChannelSelection().first;
    const casa::uInt startChan = itsSelector->getChannelSelection().second;

    if (hasMore()) {
        if (nChan == 1) {
            return nextSingle(startChan);
        } else {
            return nextMultiple(nChan, startChan);
        }
    } else {
        return false;
    }
}

casa::Bool UVChannelConstDataIterator::nextSingle(const casa::uInt chan)
{
    askap::cp::common::VisChunk::ShPtr chunk = itsReceiver->pop(chan);

    // If a null pointer is returned this indicates end-of-stream has been
    // received and there are no more data is expected.
    if (chunk.get() == 0) {
        return false;
    }

    itsConstAccessor.reset(new UVChannelConstDataAccessor(chunk));
    return true;
}

casa::Bool UVChannelConstDataIterator::nextMultiple(const casa::uInt nChan, const casa::uInt startChan)
{
    ASKAPASSERT(nChan > 1);

    // First get the starting channel data. This will be used as a basis for building
    // the larger VisChunk
    askap::cp::common::VisChunk::ShPtr golden = itsReceiver->pop(startChan);
    ASKAPASSERT(golden->nChannel() == 1);

    // If a null pointer is returned this indicates end-of-stream has been
    // received and there are no more data is expected.
    if (golden.get() == 0) {
        return false;
    }

    const casa::uInt nRow = golden->nRow();
    const casa::uInt nPol = golden->nPol();

    // Create new containers
    casa::Cube<casa::Complex> vis(nRow, nChan, nPol);
    casa::Cube<casa::Bool> flag(nRow, nChan, nPol);
    casa::Vector<casa::Double> freq(nChan);

    // Populate the new containers
    askap::cp::common::VisChunk::ShPtr chunk;

    for (casa::uInt chan = 0; chan < nChan; ++chan) {
        if (chan == 0) {
            chunk = golden;
        } else {
            // The loop uses zero-based channel indexing (for indexing into the cube) while
            // the channel number is 1 based

            chunk = itsReceiver->pop(chan + startChan);

            // Read forward, discarding VisChunks if needed, to ensure times align
            // TODO: If the golden vischunk is older than the one read here this is not
            // handled. Should handle it!! It is caught below however in the ASKAPCHECKs.
            while ((chunk.get() != 0) && (chunk->time().getTime() < golden->time().getTime())) {
                chunk = itsReceiver->pop(chan + startChan);
            }

            if (chunk.get() == 0) {
                return false;
            }

            ASKAPCHECK(chunk->time() == golden->time(), "VisChunk differs in time");
            ASKAPCHECK(chunk->nRow() == golden->nRow(), "VisChunk differs in nRow");
            ASKAPCHECK(chunk->nPol() == golden->nPol(), "VisChunk differs in nPol");
            ASKAPCHECK(chunk->nChannel() == 1, "VisChunk nChannel != 1");
            ASKAPCHECK(chunk->antenna1().size() == golden->antenna1().size(), "VisChunk differs in antenna1 vector size");
            ASKAPCHECK(chunk->antenna2().size() == golden->antenna2().size(), "VisChunk differs in antenna2 vector size");
            ASKAPCHECK(chunk->beam1().size() == golden->beam1().size(), "VisChunk differs in beam1 vector size");
            ASKAPCHECK(chunk->beam2().size() == golden->beam1().size(), "VisChunk differs in beam2 vector size");
            ASKAPCHECK(chunk->beam1PA().size() == golden->beam1PA().size(), "VisChunk differs in beam1 vector size");
            ASKAPCHECK(chunk->beam2PA().size() == golden->beam1PA().size(), "VisChunk differs in beam2 vector size");
            ASKAPCHECK(chunk->stokes().size() == golden->stokes().size(), "VisChunk differs in stokes vector size");
        }

        freq(chan) = chunk->frequency()(0);

        for (casa::uInt row = 0; row < nRow; ++row) {
            for (casa::uInt pol = 0; pol < nPol; ++pol) {
                vis(row, chan, pol) = chunk->visibility()(row, 0, pol);
                flag(row, chan, pol) = chunk->flag()(row, 0, pol);
            }
        }
    }

    golden->resize(vis, flag, freq);
    itsConstAccessor.reset(new UVChannelConstDataAccessor(golden));
    return true;
}
