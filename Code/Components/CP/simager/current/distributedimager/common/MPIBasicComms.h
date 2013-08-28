/// @file MPIBasicComms.h
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

#ifndef ASKAP_CP_MPIBASICCOMMS_H
#define ASKAP_CP_MPIBASICCOMMS_H

// Include own header file first
#include "IBasicComms.h"

// System includes
#include <string>
#include <vector>
#include <mpi.h>

// ASKAPsoft includes
#include "messages/IMessage.h"

namespace askap {
namespace cp {

/// @brief An an instance of IBasicComms providing MPI based communications
/// functionality.
class MPIBasicComms : public IBasicComms
{
public:
    /// @brief Constructor
    ///
    /// @param[in] argc argc as passed to main().
    /// @param[in] argv argv as passed to main().
    MPIBasicComms(int argc, char *argv[]);

    /// @brief Destructor
    virtual ~MPIBasicComms();

    /// @copydoc IBasicComms::getId()
    virtual int getId(void);

    /// @copydoc IBasicComms::getNumNodes()
    virtual int getNumNodes(void);

    /// @copydoc IBasicComms::abort()
    virtual void abort(void);

    /// @copydoc IBasicComms::sendMessage()
    void sendMessage(const IMessage& msg, int dest);

    /// @copydoc IBasicComms::receiveMessage()
    void receiveMessage(IMessage& msg, int source);

    /// @copydoc IBasicComms::receiveMessageAnySrc(IMessage&)
    void receiveMessageAnySrc(IMessage& msg);

    /// @copydoc IBasicComms::receiveMessageAnySrc(IMessage&,int&)
    void receiveMessageAnySrc(IMessage& msg, int& actualSource);

    /// @copydoc IBasicComms::sendMessageBroadcast()
    void sendMessageBroadcast(const IMessage& msg);

    /// @copydoc IBasicComms::receiveMessageBroadcast()
    void receiveMessageBroadcast(IMessage& msg, int root);

private:
    /// @brief MPI_Send a raw buffer to the specified destination
    /// process.
    ///
    /// @param[in] buf a pointer to the buffer to send.
    /// @param[in] size    the number of bytes to send.
    /// @param[in] dest    the id of the process to send to.
    /// @param[in] tag the MPI tag to be used in the communication.
    void send(const void* buf, size_t size, int dest, int tag);

    /// @brief MPI_Recv a raw buffer from the specified source process.
    ///
    /// @param[out] buf a pointer to the buffer to receive data into.
    /// @param[in] size    the number of bytes to receive.
    /// @param[in] source  the id of the process to receive from.
    /// @param[in] tag the MPI tag to be used in the communication.
    /// @param[out] status MPI_Status structure returned by the call to
    ///                     MPI_Recv()
    void receive(void* buf, size_t size, int source, int tag, MPI_Status& status);

    /// @brief MPI_Bcast a raw buffer.
    ///
    /// @param[in,out] buf    data buffer.
    /// @param[in] size       number of bytes to broadcast.
    /// @param[in] root       id of the root process.
    void broadcast(void* buf, size_t size, int root);

    // Check for error status and handle accordingly
    void checkError(const int error, const std::string location);

    // Add a byte offset to the  specified pointer, returning the result
    void* addOffset(const void *ptr, size_t offset);

    // Root for broadcasts
    static const int itsRoot = 0;

    // Specific MPI Communicator for this class
    MPI_Comm itsCommunicator;

    // MPE event types
    std::vector<int> itsMpeEvents;

    // No support for assignment
    MPIBasicComms& operator=(const MPIBasicComms& rhs);

    // No support for copy constructor
    MPIBasicComms(const MPIBasicComms& src);
};

}
}

#endif
