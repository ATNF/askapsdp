/// @file UVChannelPublisher.h
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

#ifndef ASKAP_CP_CHANNELS_UVCHANNELPUBLISHER_H
#define ASKAP_CP_CHANNELS_UVCHANNELPUBLISHER_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "cpcommon/VisChunk.h"
#include "Blob/BlobOBufVector.h"
#include "Blob/BlobOStream.h"

// Local package includes
#include "uvchannel/UVChannelConnection.h"

namespace askap {
namespace cp {
namespace channels {

    class UVChannelPublisher {

        public:
            UVChannelPublisher(const std::string& brokerURI,
                    const std::string& topicPrefix);

            ~UVChannelPublisher();

            void publish(const askap::cp::common::VisChunk& data, const int channel);

        private:
            // Connection to the channel
            UVChannelConnection itsConn;

            // Prefix for topics
            const std::string itsTopicPrefix;

            // Buffer for serialising messages
            std::vector<unsigned char> itsBuffer;

            // Output buffer vector
            LOFAR::BlobOBufVector<unsigned char> itsObv;

            // Blob output stream
            LOFAR::BlobOStream itsOut;

            // No support for assignment
            UVChannelPublisher& operator=(const UVChannelPublisher& rhs);

            // No support for copy constructor
            UVChannelPublisher(const UVChannelPublisher& src);
    };

};
};
};

#endif
