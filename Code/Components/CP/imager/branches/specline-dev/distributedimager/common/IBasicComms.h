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

            /// @brief Send a message to the specified destination.
            ///
            /// @param[in]  msg     the message to send.
            /// @param[in]  dest    the destination to send the message to.
            virtual void sendMessage(const IMessage& msg, int dest) = 0;

            /// @brief Receive a message of the specified type from the
            /// specified source process.
            ///
            /// @param[out] msg the message of the type you would like to 
            ///                 receive. Note this message will be overwritten
            ///                 with the contents of the received message.
            /// @param[in]  source  the id of the process to receive the
            ///                     message from.
            virtual void receiveMessage(IMessage& msg, int source) = 0;

            /// @brief Receive a message of the specified type from any source.
            ///
            /// @param[out] msg the message of the type you would like to 
            ///                 receive. Note this message will be overwritten
            ///                 with the contents of the received message.
            virtual void receiveMessageAnySrc(IMessage& msg) = 0;

            /// @brief Receive a message of the specified type from any source.
            ///
            /// @param[out] msg the message of the type you would like to 
            ///                 receive. Note this message will be overwritten
            ///                 with the contents of the received message.
            /// @param[out]  actualSource   the id of the process which actually
            ///                             sent the message.
            virtual void receiveMessageAnySrc(IMessage& msg, int& actualSource) = 0;

            /// @brief Broadcast a message to all processes.
            ///
            /// @param[in]  msg     the message to send.
            virtual void sendMessageBroadcast(const IMessage& msg) = 0;

            /// @brief Receive a message of the specified type from the
            /// specified source process.
            ///
            /// @param[out] msg the message of the type you would like to 
            ///                 receive. Note this message will be overwritten
            ///                 with the contents of the received message.
            /// @param[in]  root    the id of the process from which the
            ///                     broadcast originated.
            virtual void receiveMessageBroadcast(IMessage& msg, int root) = 0;
    };

};
};

#endif
