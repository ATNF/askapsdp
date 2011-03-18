/// @file EventConsumer.h
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

#ifndef ASKAP_CP_EVENTCHANNEL_EVENTCONSUMER_H
#define ASKAP_CP_EVENTCHANNEL_EVENTCONSUMER_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"
#include "cms/Session.h"
#include "cms/MapMessage.h"
#include "cms/MessageConsumer.h"
#include "cms/MessageListener.h"

// Local package includes
#include "eventchannel/EventMessage.h"
#include "eventchannel/IEventListener.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief The EventConsumer allows event messages to be received from a
/// given detination.
/// @ingroup eventchannel
class EventConsumer : protected cms::MessageListener {
    public:

        /// @brief Destructor
        ~EventConsumer();

        /// @brief Receive one message.
        /// This method will block until a message is received.
        /// @return a shared pointer to an EventMessage.
        EventMessageSharedPtr receive(void);

        /// Timeout of zero does a  non-blocking receive, while
        /// a negative value will block indefinetla

        /// @brief Receive one message.
        /// This method provides a timeout value, with the unit being milliseconds.
        /// If the timeout value is zero the call will be non-blocking. If
        /// the timeout value is greater then zero the call will block for
        /// that long or until a message arrives. If the timeout value is less
        /// then zero (e.g. -1) then the call will have the same behaviour as
        /// receive(void); that is will block until a message arrives.
        ///
        /// @param[in] timeout timeout in milliseconds. See above for detail.
        ///
        /// @return a shared pointer to an EventMessage.
        EventMessageSharedPtr receive(const int timeout);

        /// @brief Sets the EventListener that this class will send events to.
        /// In order to unset the listener, call this method and pass a null
        /// pointer.
        ///
        /// @note This class does not take ownership (in terms of object lifecycle)
        /// of the event listener.
        ///
        /// @param[in] listener pointer to the MessageListener that this class
        /// will send notifications to.
        void setEventListener(IEventListener* listener);

        /// @brief Gets the EventListener that this class will send event to.
        /// @return pointer to the MessageListener that this class will send
        /// events to.
        IEventListener* getEventListener(void);

    protected:

        /// @brief This is an implementation concept. It is the method
        /// via which the client (i.e. this class) is delivered new messages
        /// from the event channel.
        /// @internal
        virtual void onMessage(const cms::Message *message);

    private:

        /// @brief Constructor
        /// @note EventChannelConnection accesses this constructor as a friend
        EventConsumer(cms::Session& session, cms::MessageConsumer* consumer);

        // No support for assignment
        EventConsumer& operator=(const EventConsumer& rhs);

        // No support for copy constructor
        EventConsumer(const EventConsumer& src);

        // ActiveMQ CMS session reference (managed by the EventChannelConnection)
        cms::Session& itsSession;

        // ActiveMQ CMS message consumer
        boost::scoped_ptr<cms::MessageConsumer> itsMessageConsumer;

        // Mutex used to synchronize access to itsMailbox
        boost::mutex itsMutex;

        // Condition variable user for synchronisation between threads calling
        // receive and thread(s) in onMessage()
        boost::condition itsCondVar;

        // Mailbox to pass a message between the thread calling onMessage() and
        // the thread calling receive()
        cms::MapMessage* itsMailbox;

        IEventListener* itsEventListener;

        // Some private functions should have "package" level access. Since
        // C++ does not provide this, use friend to achieve it.
        friend class EventChannelConnection;
};

/// Short cut for shared pointer to an EventConsumer
typedef boost::shared_ptr<EventConsumer> EventConsumerSharedPtr;

};
};
};

#endif
