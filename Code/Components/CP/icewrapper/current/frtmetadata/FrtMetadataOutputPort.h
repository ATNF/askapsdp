/// @file FrtMetadataOutputPort.h
/// @details
/// This file has been converted from Ben's MetadataOutputPort and is very similar but
/// just works with a different type (simple map of ints instead of TosMetadata 
/// object). Perhaps we can refactor the class hierarchy to avoid duplication of
/// code. This class is intended to be used in communication with the utility
/// controlling the BETA fringe rotator and/or DRx-based delay tracking.
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
/// @author Max Voronkov <Max.Voronkov@csiro.au>

#ifndef ASKAP_CP_ICEWRAPPER_FRTMETADATAOUTPUTPORT_H
#define ASKAP_CP_ICEWRAPPER_FRTMETADATAOUTPUTPORT_H

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"

// CP ice interfaces
#include "TypedValues.h"

// Local package includes
#include "iceutils/OutputPort.h"

namespace askap {
namespace cp {
namespace icewrapper {

/// @brief A class which can be used to send map of integers to
/// an IceStorm topic.
/// @details
/// This class has been converted from Ben's MetadataOutputPort and is very similar but
/// just works with a different type (simple map of ints instead of TosMetadata 
/// object). Perhaps we can refactor the class hierarchy to avoid duplication of
/// code. This class is intended to be used in communication with the utility
/// controlling the BETA fringe rotator and/or DRx-based delay tracking.
/// @ingroup frtmetadata
class FrtMetadataOutputPort {
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
        FrtMetadataOutputPort(const std::string& locatorHost,
                              const std::string& locatorPort,
                              const std::string& topicManager,
                              const std::string& topic);

        /// @brief Destructor
        ~FrtMetadataOutputPort();

        /// @brief Send a TypedValueMap message via this port.
        ///
        /// @param[in] message the message to send.
        void send(const std::map<std::string, int> & message);

    private:
        // Type of the output port (templated)
        typedef OutputPort < askap::interfaces::TypedValueMap,
        askap::interfaces::datapublisher::ITypedValueMapPublisherPrx >
        OutputPortType;

        // Pointer to the output port instance
        boost::scoped_ptr<OutputPortType> itsOutputPort;

        // The proxy via which messages are published
        askap::interfaces::datapublisher::ITypedValueMapPublisherPrx itsProxy;
};

}
}
}

#endif // #ifndef ASKAP_CP_ICEWRAPPER_FRTMETADATAOUTPUTPORT_H

