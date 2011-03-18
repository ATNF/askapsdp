/// @file UVChannelConsumer.h
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

#ifndef ASKAP_CP_CHANNELS_UVCHANNELCONSUMER_H
#define ASKAP_CP_CHANNELS_UVCHANNELCONSUMER_H

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "Common/ParameterSet.h"

// Local package includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/IUVChannelListener.h"
#include "uvchannel/ConsumerActual.h"

namespace askap {
namespace cp {
namespace channels {

    /// @brief Consumer class. Wraps the process of consuming from the uv-channel,
    /// regardless of which broker is responsible for the particular channelName
    /// and/or channel number.
    /// @ingroup uvchannel
    class UVChannelConsumer {

        public:
            /// Constructor.
            /// @param[in] listener pointer to the IUVChannelListener that this class
            /// will send notifications to.
            UVChannelConsumer(const LOFAR::ParameterSet& parset,
                    const std::string& channelName,
                    IUVChannelListener* listener);

            /// Constructor.
            /// @param[in] listener pointer to the IUVChannelListener that this class
            /// will send notifications to.
            UVChannelConsumer(const  UVChannelConfig& channelConfig,
                    const std::string& channelName,
                    IUVChannelListener* listener);

            /// Destructor.
            ~UVChannelConsumer();

            /// @brief Add a subscription.
            ///
            /// @param[in] channel  the channel number to subscribe to
            void addSubscription(const int channel);

            /// @brief Remove a subscription.
            ///
            /// @param[in] channel  the channel number to unsubscribe from
            void removeSubscription(const int channel);

        private:

            // Get the broker specific consumer for the specified broker
            boost::shared_ptr<ConsumerActual> getActual(const std::string& brokerId);

            // Mapping between channel names/number and brokers
            const UVChannelConfig itsConfig;

            // Channel name (used for lookup in the parset)
            const std::string itsChannelName;

            // Once messages are recieved and converted to a VisChunk, a callback
            // to the object registered here is made
            IUVChannelListener* itsVisListener;

            // Connection map
            std::map< std::string, boost::shared_ptr<ConsumerActual> > itsConnectionMap;

            // No support for assignment
            UVChannelConsumer& operator=(const UVChannelConsumer& rhs);

            // No support for copy constructor
            UVChannelConsumer(const UVChannelConsumer& src);
    };

};
};
};

#endif
