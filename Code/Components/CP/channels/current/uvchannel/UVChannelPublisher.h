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
#include <map>

// ASKAPsoft includes
#include "cpcommon/VisChunk.h"
#include "Blob/BlobOBufVector.h"
#include "Blob/BlobOStream.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/UVChannelConnection.h"

namespace askap {
namespace cp {
namespace channels {

    class UVChannelPublisher {

        public:
            UVChannelPublisher(const LOFAR::ParameterSet& parset, const std::string& channelName);

            ~UVChannelPublisher();

            void publish(const askap::cp::common::VisChunk& data, const int channel);

        private:

            // Returns the connection from the connection map, creating a new one
            // if there is not already a connection for the specified broker
            boost::shared_ptr<UVChannelConnection> getConnection(const std::string& brokerId);

            // Mapping between channel names/number and brokers
            const UVChannelConfig itsConfig;

            // Channel name (used for lookup in the parset)
            const std::string itsChannelName;

            // Buffer for serialising messages
            std::vector<unsigned char> itsBuffer;

            // Output buffer vector
            LOFAR::BlobOBufVector<unsigned char> itsObv;

            // Blob output stream
            LOFAR::BlobOStream itsOut;

            // Connection map
            std::map< std::string, boost::shared_ptr<UVChannelConnection> > itsConnectionMap;

            // No support for assignment
            UVChannelPublisher& operator=(const UVChannelPublisher& rhs);

            // No support for copy constructor
            UVChannelPublisher(const UVChannelPublisher& src);
    };

};
};
};

#endif
