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
    const casa::uInt startChan = itsSelector->getChannelSelection().first;
    const casa::uInt nChan = itsSelector->getChannelSelection().second;
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
    return itsReceiver->hasMore();
}

casa::Bool UVChannelConstDataIterator::next()
{
    if (hasMore()) {
        boost::shared_ptr<askap::cp::common::VisChunk> chunk = itsReceiver->next();

        // If a null pointer is returned this indicates end-of-stream has been
        // received and there are no more data is expected.
        if (chunk.get() == 0) {
            return false;
        }
        itsConstAccessor.reset(new UVChannelConstDataAccessor(chunk));
        return true;
    } else {
        return false;
    }
}
