/// @file MetadataReceiver.h
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

#ifndef ASKAP_CP_METADATARECEIVER_H
#define ASKAP_CP_METADATARECEIVER_H

// System includes
#include <string>

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"

// CP ice interfaces
#include "TypedValues.h"

namespace askap {
    namespace cp {

        class MetadataReceiver :
            virtual public askap::interfaces::datapublisher::ITimeTaggedTypedValueMapPublisher
        {
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
                MetadataReceiver(const std::string& locatorHost,
                        const std::string& locatorPort,
                        const std::string& topicManager,
                        const std::string& topic,
                        const std::string& adapterName);

                /// @brief Destructor
                ~MetadataReceiver();

                virtual void receive(const askap::interfaces::TimeTaggedTypedValueMap& msg) = 0;

            private:
                virtual void publish(
                        const askap::interfaces::TimeTaggedTypedValueMap& msg,
                        const Ice::Current& c);

                // An Ice proxy to the object this class registers
                // (what happens to be itself)
                Ice::ObjectPrx itsProxy;

                // Proxy to the topic manager
                IceStorm::TopicPrx itsTopicPrx;

        };

    };
};

#endif
