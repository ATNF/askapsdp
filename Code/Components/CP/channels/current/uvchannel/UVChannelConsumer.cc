/// @file UVChannelConsumer.cc
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
#include "UVChannelConsumer.h"

// Include package level header file
#include "askap_channels.h"

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Blob/BlobOStream.h"
#include "Blob/BlobOBufVector.h"
#include "Common/ParameterSet.h"
#include "cms/Message.h"
#include "cms/BytesMessage.h"

// Local package includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/PublisherActual.h"

ASKAP_LOGGER(logger, ".UVChannelConsumer");

// Using
using namespace std;
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::channels;

UVChannelConsumer::UVChannelConsumer(const LOFAR::ParameterSet& parset,
        const std::string& channelName,
        IUVChannelListener* listener)
    : itsConfig(parset), itsChannelName(channelName), itsVisListener(listener)
{
    if (!listener) {
        ASKAPTHROW(AskapError, "Visibilities listener is null");
    }
}

UVChannelConsumer::UVChannelConsumer(const  UVChannelConfig& channelConfig,
        const std::string& channelName,
        IUVChannelListener* listener)
    : itsConfig(channelConfig), itsChannelName(channelName), itsVisListener(listener)
{
    if (!listener) {
        ASKAPTHROW(AskapError, "Visibilities listener is null");
    }
}

UVChannelConsumer::~UVChannelConsumer()
{
}

void UVChannelConsumer::addSubscription(const int channel)
{
    // Get topic and broker id
    const string topic = itsConfig.getTopic(itsChannelName, channel);
    const string brokerId = itsConfig.getBrokerId(itsChannelName, channel);
    getActual(brokerId)->addSubscription(topic);
}

void UVChannelConsumer::removeSubscription(const int channel)
{
    // Get topic and broker id
    const string topic = itsConfig.getTopic(itsChannelName, channel);
    const string brokerId = itsConfig.getBrokerId(itsChannelName, channel);
    getActual(brokerId)->removeSubscription(topic);
}

boost::shared_ptr<ConsumerActual> UVChannelConsumer::getActual(const std::string& brokerId)
{
    map< std::string, boost::shared_ptr<PublisherActual> >::iterator it;
    if (itsConnectionMap.find(brokerId) == itsConnectionMap.end()) {
        // Need to create the connecton
        stringstream ss;
        ss << "tcp://" << itsConfig.getHost(brokerId) << ":" << itsConfig.getPort(brokerId);
        ss << "&conection.useAsyncSend=true";
        ss << "&turboBoost=true";
        ss << "&socketBufferSize=16384";
        itsConnectionMap[brokerId] = boost::shared_ptr<ConsumerActual>(new ConsumerActual(ss.str(), itsVisListener));
    }

    return itsConnectionMap[brokerId];
}
