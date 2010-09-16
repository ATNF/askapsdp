/// @file IEventMessage.h
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

#ifndef ASKAP_CP_EVENTCHANNEL_IEVENTMESSAGE_H
#define ASKAP_CP_EVENTCHANNEL_IEVENTMESSAGE_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "cms/Message.h"

namespace askap {
namespace cp {
namespace eventchannel {

class IEventMessage {
    public:
        /// @brief Destructor
        virtual ~IEventMessage();
        
    private:
        virtual cms::Message* getCmsMessage(void) = 0;

        // Some private functions should have "package" level access. Since
        // C++ does not provide this, use friend to achieve it.
        friend class EventChannelConnection;
        friend class EventProducer;
        friend class EventConsumer;
};

/// Short cut for shared pointer to an IEventMessage
typedef boost::shared_ptr<IEventMessage> IEventMessageSharedPtr;

};
};
};

#endif
