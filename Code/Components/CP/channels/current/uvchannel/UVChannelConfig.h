/// @file UVChannelConfig.h
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

#ifndef ASKAP_CP_CHANNELS_UVCHANNELCONFIG_H
#define ASKAP_CP_CHANNELS_UVCHANNELCONFIG_H

// System includes
#include <string>

// ASKAPsoft includes
#include "Common/ParameterSet.h"

// Local package includes

namespace askap {
namespace cp {
namespace channels {

    /// @brief Encapsulates the mappings between channel name, channel number and the broker.
    class UVChannelConfig {

        public:
            /// @brief Constructor.
            /// @param[in] parset a parset which describes the mappings between channel name,
            /// channel number and the broker.
            UVChannelConfig(const LOFAR::ParameterSet& parset);

            /// @brief Given a channel name and channel number, return the brokerId
            /// @param[in] name channel name
            /// @param[in] chan channel number
            /// @return the broker id
            std::string getBrokerId(const std::string& name, const int chan) const;

            /// @brief Given a broker id, return the name of the host it is deployed on
            /// @param[in] brokerId the broker id
            /// @return the name of the host the broker is deployed on
            std::string getHost(const std::string& brokerId) const;

            /// @brief Given a broker id, return the network port number
            /// @param[in] brokerId the broker id
            /// @return the network port number
            int getPort(const std::string& brokerId) const;

            /// @brief Given a channel name and channel number, return the name of the topic
            /// @param[in] name channel name
            /// @param[in] chan channel number
            /// @return the topic name to use for publishing or subscribing
            std::string getTopic(const std::string& name, const int chan) const;

        private:

            template <class T>
            T strTo(const std::string& str) const;

            const LOFAR::ParameterSet itsParset;
    };

};
};
};

#endif
