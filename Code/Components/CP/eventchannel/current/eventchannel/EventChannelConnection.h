/// @file EventChannelConnection.h
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

#ifndef ASKAP_CP_EVENTCHANNEL_EVENTCHANNELCONNECTION_H
#define ASKAP_CP_EVENTCHANNEL_EVENTCHANNELCONNECTION_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "cms/Connection.h"
#include "cms/Session.h"
#include "cms/ExceptionListener.h"

// Local package includes
#include "EventProducer.h"
#include "EventConsumer.h"
#include "EventDestination.h"
#include "EventMessage.h"

namespace askap {
namespace cp {
namespace eventchannel {

class EventChannelConnection : protected cms::ExceptionListener {
    public:
        static EventChannelConnection& getSingletonInstance(void);

        static EventChannelConnection& createSingletonInstance(const std::string& brokerURI);

        ~EventChannelConnection();

        EventProducerSharedPtr createEventChannelProducer(EventDestination& dest);

        EventConsumerSharedPtr createEventChannelConsumer(EventDestination& dest);

        EventDestinationSharedPtr createEventDestination(const std::string& name,
                EventDestination::DestinationType type);

        EventMessageSharedPtr createEventMessage(void);

    protected:

        virtual void onException(const cms::CMSException &ex);

    private:

        EventChannelConnection(const std::string& brokerURI);

        // No support for assignment
        EventChannelConnection& operator=(const EventChannelConnection& rhs);

        // No support for copy constructor
        EventChannelConnection(const EventChannelConnection& src);

        // Singleton instance of this class
        static EventChannelConnection *itsInstance;

        // ActiveMQ Connection
        boost::scoped_ptr<cms::Connection> itsConnection;

        // ActiveMQ Session
        boost::scoped_ptr<cms::Session> itsSession;

};

};
};
};

#endif
