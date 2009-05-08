/// @file IBasicComms.h
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

#ifndef ASKAP_CP_IBASICCOMMS_H
#define ASKAP_CP_IBASICCOMMS_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include <messages/IMessage.h>

namespace askap {
    namespace cp {

        /// @brief An interface defining communications functionality required
        /// for the distributed imager.
        class IBasicComms
        {
            public:
                /// @brief Destructor.
                virtual ~IBasicComms();

                /// @brief Returns the Id of the process. This allows the process to be
                ///  uniquely identified within the group of collaborating processes.
                ///
                /// @return the Id of the process.
                virtual int getId(void) = 0;

                /// @brief Returns the number of nodes involved in the collaboration.
                ///
                /// @return the number of nodes involved in the collaboration.
                virtual int getNumNodes(void) = 0;

                /// @brief Abort the collaboration and signal all processes
                /// involved to terminate.
                virtual void abort(void) = 0;

                virtual void sendMessage(const IMessage& msg, int dest) = 0;
                virtual const IMessageSharedPtr receiveMessage(IMessage::MessageType type, int source) = 0;
                virtual const IMessageSharedPtr receiveMessageAnySrc(IMessage::MessageType type) = 0;
                virtual const IMessageSharedPtr receiveMessageAnySrc(IMessage::MessageType type, int& actualSource) = 0;

                virtual void sendMessageBroadcast(const IMessage& msg) = 0;
                virtual const IMessageSharedPtr receiveMessageBroadcast(IMessage::MessageType type, int root) = 0;
        };

    };
};

#endif
