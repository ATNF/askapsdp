/// @file UVChannelPublisher.cc
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
#include "UVChannelPublisher.h"

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Blob/BlobOStream.h"
#include "Blob/BlobOBufVector.h"
#include "cpcommon/VisChunk.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/UVChannelConnection.h"

ASKAP_LOGGER(logger, ".UVChannelPublisher");

// Using
using namespace std;
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::channels;

UVChannelPublisher::UVChannelPublisher(const LOFAR::ParameterSet& parset, const std::string& channelName)
    : itsConfig(parset), itsChannelName(channelName), itsObv(itsBuffer), itsOut(itsObv)
{
}

UVChannelPublisher::~UVChannelPublisher()
{
}

void UVChannelPublisher::publish(const askap::cp::common::VisChunk& data,
        const int channel)
{
    // Get topic and broker id
    const string topic = itsConfig.getTopic(itsChannelName, channel);
    const string brokerId = itsConfig.getBrokerId(itsChannelName, channel);

    // Reset the blob objects (need to reuse them for performance reasons)
    itsObv.clear();
    itsOut.clear();

    // Serialize
    itsOut.putStart("VisChunk", 1);
    itsOut << data;
    itsOut.putEnd();

    // Send
    getConnection(brokerId)->sendByteMessage(itsObv.getBuffer(), itsObv.size(), topic);
}

boost::shared_ptr<UVChannelConnection> UVChannelPublisher::getConnection(const std::string& brokerId)
{
    map< std::string, boost::shared_ptr<UVChannelConnection> >::iterator it;
    if (itsConnectionMap.find(brokerId) == itsConnectionMap.end()) {
        // Need to create the connecton
        stringstream ss;
        ss << "tcp://" << itsConfig.getHost(brokerId) << ":" << itsConfig.getPort(brokerId);
        ss << "&conection.useAsyncSend=true";
        ss << "&turboBoost=true";
        ss << "&socketBufferSize=16384";
        ss << ")";
        itsConnectionMap[brokerId] = boost::shared_ptr<UVChannelConnection>(new UVChannelConnection(ss.str()));
    }

    return itsConnectionMap[brokerId];
}
