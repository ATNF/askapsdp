/// @file MetadataPort.h
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

#ifndef ASKAP_CP_METADATAPORT_H
#define ASKAP_CP_METADATAPORT_H

// System includes
#include <string>

// ASKAPsoft includes
#include "Ice/Ice.h"

// Ice interface includes
#include "TypedValues.h"

namespace askap {
namespace cp {

/// @brief This class acts as a port to the metadata topic. Metadata messages
/// can be "sent" using this port which will publish them to the topic
/// specified in the constructor call.
class MetadataPort {
    public:

        /// @brief Constructor.
        ///
        /// @param[in] dataset filename for the measurement set which
        ///                    will be used to source the metadata.
        /// @param[in] locatorHost hostname or IP address of the host
        ///                     where the Ice locator service is running.
        /// @param[in] locatorPort network port for the Ice locator service.
        /// @param[in] topicManager identify of the IceStorm topic manager
        ///                     within the Ice locator service.
        /// @param[in] topic    IceStorm topic to which the metadata will be
        ///                     published.
        MetadataPort(const std::string& locatorHost,
                     const std::string& locatorPort,
                     const std::string& topicManager,
                     const std::string& topic);

        /// @brief Destructor.
        ~MetadataPort();

        /// @brief Publishes the payload to the IceStorm topic specified when
        /// this object was instantiated.
        ///
        /// @param[in]  payload the payload to publish.
        void send(const askap::interfaces::TimeTaggedTypedValueMap& payload);

    private:
        // Get an IceStorm topic publisher proxy
        Ice::ObjectPrx getProxy(const std::string& topicManager,
                                const std::string& topic);

        // Ice Communicator
        Ice::CommunicatorPtr itsComm;

        // Ice proxy for the metadata stream topic
        askap::interfaces::datapublisher::ITimeTaggedTypedValueMapPublisherPrx itsMetadataStream;
};

};
};

#endif
