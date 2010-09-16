/// @file EventConsumer.cc
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

// Include own header file first
#include "EventConsumer.h"

// Include package level header file
#include "askap_eventchannel.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"
#include "boost/thread/xtime.hpp"
#include "cms/Session.h"
#include "cms/Message.h"
#include "cms/MapMessage.h"
#include "cms/MessageConsumer.h"

// Local package includes
#include "eventchannel/EventMessage.h"

ASKAP_LOGGER(logger, ".EventConsumer");

using namespace askap;
using namespace askap::cp;
using namespace askap::cp::eventchannel;

EventConsumer::EventConsumer(cms::Session& session, cms::MessageConsumer* consumer)
        : itsSession(session), itsMessageConsumer(consumer), itsMailbox(0)
{
    consumer->setMessageListener(this);
}

EventConsumer::~EventConsumer()
{
    itsMessageConsumer->close();
    itsMessageConsumer.reset();
    delete itsMailbox;
}

EventMessageSharedPtr EventConsumer::receive(void)
{
    // -1 implies wait as long as necessary
    return receive(-1);
}

EventMessageSharedPtr EventConsumer::receive(const int timeout)
{
    /// TODO: The non-blocking case is really blocking, it still
    /// needs to aquire the mutex. Use a try_lock to ensure non-blocking
    /// functionality.

    // Determine when to sleep to if timeout is set
    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);

    // Note: The timeout parameter is in milliseconds
    if (timeout > 0) {
        const long timeout_usec = timeout * 1000;
        const long NANOSECONDS_PER_MICROSECOND = 1000;
        const long MICROSECONDS_PER_SECOND = 1000000;

        if (timeout_usec > MICROSECONDS_PER_SECOND) {
            const long sec = timeout_usec / MICROSECONDS_PER_SECOND;
            xt.sec += sec;
        }

        xt.nsec += (timeout_usec % MICROSECONDS_PER_SECOND) * NANOSECONDS_PER_MICROSECOND;
    }

    boost::mutex::scoped_lock lock(itsMutex);

    while (!itsMailbox) {
        if (timeout == 0) { // Non blocking case
            return EventMessageSharedPtr();
        } else if (timeout > 0) { // Timeout case
            const bool timeoutReached = !(itsCondVar.timed_wait(lock, xt));

            if (timeoutReached) {
                return EventMessageSharedPtr();
            }
        } else { // Wait as long as necessary case
            itsCondVar.wait(lock);
        }
    }

    // If we get here, mailbox is populated and lock is held
    ASKAPDEBUGASSERT(itsMailbox);
    EventMessageSharedPtr message(new EventMessage(itsMailbox));
    itsMailbox = 0;
    return message;
}

void EventConsumer::onMessage(const cms::Message *message)
{
    const cms::MapMessage* mapMessage =
        dynamic_cast<const cms::MapMessage*>(message);

    if (!mapMessage) {
        ASKAPLOG_WARN_STR(logger, "Message of non map type received on event channel");
        return;
    }

    // clone the message so ownership can be transferred the the EventMessage
    // object created and evenatually returned
    cms::MapMessage* clone = dynamic_cast<cms::MapMessage*>(mapMessage->clone());
    ASKAPDEBUGASSERT(clone);

    // Sleep while the mailbox is full
    boost::mutex::scoped_lock lock(itsMutex);

    while (itsMailbox) {
        itsCondVar.wait(lock);
    }

    // Now the mailbox is empty, put a message in it and wakeup waiters
    itsMailbox = clone;
    itsCondVar.notify_one();
}
