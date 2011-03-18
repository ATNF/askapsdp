/// @file IEventListener.h
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

#ifndef ASKAP_CP_EVENTCHANNEL_IEVENTLISTENER_H
#define ASKAP_CP_EVENTCHANNEL_IEVENTLISTENER_H

// System includes
#include <string>

// Local package includes
#include "eventchannel/EventMessage.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief The IEventListener allows event messages to be asynchronously
/// received.
/// @ingroup eventchannel
class IEventListener {
    public:

        /// @brief Destructor
        virtual ~IEventListener();

        /// @brief Called asynchronously when a new event message is received.
        ///
        /// @param[in] message a shared pointer to the message. The recipient
        /// now has ownership of this message via the shared pointer.
        virtual void onMessage(const EventMessageSharedPtr message) = 0;
};

};
};
};

#endif
