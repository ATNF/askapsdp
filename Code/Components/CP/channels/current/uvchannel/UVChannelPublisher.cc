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

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Blob/BlobOStream.h"
#include "Blob/BlobOBufVector.h"
#include "cpcommon/VisChunk.h"

// Local package includes

ASKAP_LOGGER(logger, ".UVChannelPublisher");

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::channels;

UVChannelPublisher::UVChannelPublisher(const std::string& brokerURI,
                                        const std::string& topicPrefix)
    : itsConn(brokerURI), itsTopicPrefix(topicPrefix)
{
}

UVChannelPublisher::~UVChannelPublisher()
{
}

void UVChannelPublisher::publish(const askap::cp::common::VisChunk& data,
        const int channel)
{
    std::stringstream ss;
    ss << itsTopicPrefix << channel;

    // Expand size. Size of increment for Blob BufVector storage.
    // Too small and there is lots of overhead in expanding the vector.
    const unsigned int expandSize = 1024 * 1024;

    // Serialize
    LOFAR::BlobOBufVector<unsigned char> obv(itsBuffer, expandSize);
    LOFAR::BlobOStream out(obv);
    out.putStart("VisChunk", 1);
    out << data;
    out.putEnd();

    // Send
    itsConn.sendByteMessage(&itsBuffer[0], itsBuffer.size() * sizeof(unsigned char), ss.str());
}

