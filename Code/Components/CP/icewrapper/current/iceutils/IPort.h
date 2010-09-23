/// @file IPort.h
///
/// @copyright (c) 2009 CSIRO
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

#ifndef ASKAP_CP_ICEWRAPPER_IPORT_H
#define ASKAP_CP_ICEWRAPPER_IPORT_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"

namespace askap {
namespace cp {
namespace icewrapper {

/// @brief An interface for the InputPort and Outport classes.
///
/// This specifies the interface to publish/subscribe ports.
/// @ingroup iceutils
class IPort {
    public:

        /// @brief Destructor.
        virtual ~IPort() {};

        /// @brief Direction enumeration
        enum Direction {
            /// This is an input port
            IN,
            /// This is an output port
            OUT
        };

        /// @brief Returns the direction of this port, either input
        /// or output.
        ///
        /// @return Direction:IN or Direction:OUT.
        virtual Direction getDirection(void) const = 0;

        /// @brief Attach the port instance to a topic, where the topic
        /// is obtained from the specified topic manager.
        ///
        /// @param[in] topic the name of the topic to attach the port to.
        /// @param[in] topicManager the identity of the topic manager from
        ///                         where the topic subscription should be
        ///                         requested.
        virtual void attach(const std::string& topic,
                            const std::string& topicManager) = 0;

        /// @brief Detach from the attached topic.
        /// This has no effect if a call to attach() has not yet been made,
        /// or if detach has already been called.
        virtual void detach(void) = 0;

        /// Shared pointer definition
        typedef boost::shared_ptr<IPort> ShPtr;
};

}
}
}

#endif
