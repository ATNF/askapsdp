/// @file UVChannelReceiver.cc
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
#include "UVChannelReceiver.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "cpcommon/VisChunk.h"
#include "boost/shared_ptr.hpp"

// Local includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/UVChannelConsumer.h"
#include "uvchannel/IUVChannelListener.h"

// Using
using namespace casa;
using namespace askap;
using namespace askap::cp::channels;

UVChannelReceiver::UVChannelReceiver(const UVChannelConfig& channelConfig,
        const std::string& channelName,
        const casa::uInt startChan,
        const casa::uInt nChan,
        const casa::uInt maxQueueSize)
    : itsMaxQueueSize(maxQueueSize)
{
    itsConsumer.reset(new UVChannelConsumer(channelConfig, channelName, this));
    for (unsigned int c = startChan; c <= nChan; ++c) {
        itsConsumer->addSubscription(c);
    }
}

UVChannelReceiver::~UVChannelReceiver()
{
    itsConsumer.reset();
}

casa::Bool UVChannelReceiver::hasMore(void) const
{
    return true;
}

boost::shared_ptr<askap::cp::common::VisChunk> UVChannelReceiver::next(void)
{
    boost::mutex::scoped_lock lock(itsMutex);
    while (itsQueue.empty()) {
        itsCondVar.wait(lock);
    }

    boost::shared_ptr<askap::cp::common::VisChunk> obj(itsQueue.front());
    itsQueue.pop_front();
    lock.unlock();

    // No need to notify producer. The producer doesn't block, instead it
    // discards messages when the queue is full

    return obj;
}

void UVChannelReceiver::onMessage(const boost::shared_ptr<askap::cp::common::VisChunk> message)
{
    // Add a pointer to the message to the back of the buffer
    boost::mutex::scoped_lock lock(itsMutex);

    if (itsQueue.size() >= itsMaxQueueSize) {
        return;
    }

    itsQueue.push_back(message);

    // Notify any waiters
    lock.unlock();
    itsCondVar.notify_all();
}
