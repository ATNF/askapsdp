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
    for (casa::uInt chan = startChan; chan < startChan+nChan; ++chan) {
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
    boost::shared_ptr<askap::cp::common::VisChunk> chunk = itsReceiver->next(chan);

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
    boost::shared_ptr<askap::cp::common::VisChunk> golden = itsReceiver->next(startChan);
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
    boost::shared_ptr<askap::cp::common::VisChunk> chunk;
    for (casa::uInt chan = 0; chan < nChan; ++chan) {
        if (chan == 0) {
            chunk = golden;
        } else {
            // The loop uses zero-based channel indexing (for indexing into the cube), but
            chunk = itsReceiver->next(chan+startChan);
            if (chunk.get() == 0) {
                return false;
            }
            ASKAPASSERT(chunk->time() == golden->time());
            ASKAPASSERT(chunk->nRow() == golden->nRow());
            ASKAPASSERT(chunk->nPol() == golden->nPol());
            ASKAPASSERT(chunk->nChannel() == 1);
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
