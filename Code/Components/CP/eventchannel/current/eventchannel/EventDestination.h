/// @file EventDestination.h
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

#ifndef ASKAP_CP_EVENTCHANNEL_EVENTDESTINATION_H
#define ASKAP_CP_EVENTCHANNEL_EVENTDESTINATION_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "cms/Destination.h"

namespace askap {
namespace cp {
namespace eventchannel {

/// @brief A Destination object encapsulates an abstraction (either topic or
/// queue) to which events may be sent of received from.
///
/// A destination may be either a topic or a queue:
/// @li Topics implement publish and subscribe semantics. When you publish a
/// message it goes to all the subscribers.
///
/// @li Queues implement load balancer semantics. A single message will be received
/// by exactly one consumer. If there are no consumers available at the time the
/// message is sent it will be kept until a consumer is available that can
/// process the message. 
class EventDestination {
    public:

        /// @brief Destination enumeration.
        enum DestinationType {
            TOPIC,
            QUEUE
        };
        
        /// @brief Destructor
        ~EventDestination();

        /// @brief returns the type of the destination.
        /// @return the type of the destination.
        DestinationType getType(void);

    private:

        /// @brief Constructor
        /// @note EventChannelConnection accesses this constructor as a friend
        EventDestination(cms::Destination* dest);

        /// @note EventChannelConnection accesses this method as a friend
        cms::Destination* getCmsDestination(void);

        // No support for assignment
        EventDestination& operator=(const EventDestination& rhs);

        // No support for copy constructor
        EventDestination(const EventDestination& src);

        // ActiveMQ CMS destination
        boost::scoped_ptr<cms::Destination> itsDestination;

        // Some private functions should have "package" level access. Since
        // C++ does not provide this, use friend to achieve it.
        friend class EventChannelConnection;
};

/// Short cut for shared pointer to an EventDestination
typedef boost::shared_ptr<EventDestination> EventDestinationSharedPtr;

};
};
};

#endif
