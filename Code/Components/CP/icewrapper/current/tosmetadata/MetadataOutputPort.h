/// @file MetadataOutputPort.h
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

#ifndef ASKAP_CP_ICEWRAPPER_METADATAOUTPUTPORT_H
#define ASKAP_CP_ICEWRAPPER_METADATAOUTPUTPORT_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "cpcommon/TosMetadata.h"

// CP ice interfaces
#include "TypedValues.h"

// Local package includes
#include "iceutils/OutputPort.h"

namespace askap {
namespace cp {
namespace icewrapper {

/// @brief A class which can be used to send instance of TosMetadata to
/// an IceStorm topic.
/// @ingroup tosmetadata
class MetadataOutputPort {
    public:
        /// @brief Constructor.
        ///
        /// @param[in] the hostname or IP-address of the locator
        ///     service (registry).
        /// @param[in] locatorPort the port number of the locator
        ///     service which is running on the host specified by
        ///     the locatorHost parameter.
        /// @param[in] topicManager the identity of the topic manager
        ///     from where the topic subscription should be requested.
        /// @param[in] topic the name of the topic to attach the port
        ///     to. This is the topic where messages wil be sent.
        MetadataOutputPort(const std::string& locatorHost,
                           const std::string& locatorPort,
                           const std::string& topicManager,
                           const std::string& topic);

        /// @brief Destructor
        ~MetadataOutputPort();

        /// @brief Send a TimeTaggedTypedValueMap message via this port.
        ///
        /// @param[in] message the message to send.
        void send(const askap::cp::TosMetadata& message);

    private:
        // Type of the output port (templated)
        typedef OutputPort < askap::interfaces::TimeTaggedTypedValueMap,
        askap::interfaces::datapublisher::ITimeTaggedTypedValueMapPublisherPrx >
        OutputPortType;

        // Pointer to the output port instance
        boost::scoped_ptr<OutputPortType> itsOutputPort;

        // The proxy via which messages are published
        askap::interfaces::datapublisher::ITimeTaggedTypedValueMapPublisherPrx itsProxy;
};

}
}
}

#endif
