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

// ASKAPsoft includes
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>

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

                /// @brief Broadcast the model to all participants.
                /// @param[in]  model   the model to broadcast.
                virtual void broadcastModel(askap::scimath::Params::ShPtr model) = 0;

                /// @brief Receive the model which has been broadcasted by the
                /// broadcastModel() call.
                ///
                /// @return the model.
                virtual askap::scimath::Params::ShPtr receiveModel(void) = 0;

                /// @brief Send normal equations to the root process.
                /// @param[in]  ne  the normal equations to send.
                /// @param[in]  id  the id of the process to send to.
                virtual void sendNE(askap::scimath::INormalEquations::ShPtr ne, int id, int count) = 0;

                /// @brief Receive the normal equations which have been sent by
                ///  a sendNE() call.
                /// @param[out] id  a reference to an integer which will be set to
                /// the id of the participant which has sent the message.
                ///
                /// @return the received normal equations.
                virtual askap::scimath::INormalEquations::ShPtr receiveNE(int& id, int& count) = 0;

                /// @brief Send a string to the indicated destination.
                /// @param[in]  string  the string to send.
                /// @param[in]  dest    the id of the destination to send to.
                virtual void sendString(const std::string& str, int dest) = 0;

                /// @brief Receive a string which has been sent by the 
                /// sendString() call.
                ///
                /// @param[in]  source  the id of the source to recieve
                ///                     the string from.
                /// @return the received string.
                virtual std::string receiveString(int source) = 0;

                /// @brief Receive a string which has been sent by the 
                /// sendString() call. This call will receive a string
                /// from any node.
                ///
                /// @param[out] source  the id of the source from which the
                ///                     string as received.
                /// @return the received string.
                virtual std::string receiveStringAny(int& source) = 0;
        };

    };
};

#endif
