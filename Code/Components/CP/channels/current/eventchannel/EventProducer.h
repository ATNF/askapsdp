/// @file EventProducer.h
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

#ifndef ASKAP_CP_EVENTCHANNEL_EVENTPRODUCER_H
#define ASKAP_CP_EVENTCHANNEL_EVENTPRODUCER_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "cms/Session.h"
#include "cms/MessageProducer.h"

// Local package includes
#include "eventchannel/EventMessage.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief The EventProducer allows event messages to be sent via the event
/// channel.
class EventProducer {
    public:

        /// @brief Destructor
        ~EventProducer();

        /// @brief Send an event message via the event channel, to the
        /// destination topic or queue which the EventProducer is registered
        /// to.
        ///
        /// @param[in] message  a reference to the message to send.
        void send(EventMessage& message);

    private:

        /// @brief Constructor
        /// @note EventChannelConnection accesses this constructor as a friend
        EventProducer(cms::Session& session, cms::MessageProducer* producer);

        // No support for assignment
        EventProducer& operator=(const EventProducer& rhs);

        // No support for copy constructor
        EventProducer(const EventProducer& src);

        // ActiveMQ CMS session reference (managed by the EventChannelConnection)
        cms::Session& itsSession;

        // ActiveMQ CMS message producer
        boost::scoped_ptr<cms::MessageProducer> itsMessageProducer;

        // Some private functions should have "package" level access. Since
        // C++ does not provide this, use friend to achieve it.
        friend class EventChannelConnection;
};

/// Short cut for shared pointer to an EventProducer
typedef boost::shared_ptr<EventProducer> EventProducerSharedPtr;

};
};
};

#endif
