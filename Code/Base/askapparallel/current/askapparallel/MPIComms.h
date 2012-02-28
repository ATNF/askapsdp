// @file MPIComms.h
///
/// @copyright (c) 2012 CSIRO
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

#ifndef ASKAP_ASKAPPARALLEL_MPICOMMS_H
#define ASKAP_ASKAPPARALLEL_MPICOMMS_H

// System includes
#include <string>
#ifdef HAVE_MPI
#include <mpi.h>
#endif

// ASKAPsoft includes

namespace askap {
namespace askapparallel {

/// @brief An an instance of IComms providing MPI based communications
/// functionality.
class MPIComms {
    public:
        /// @brief Constructor
        ///
        /// @param[in] argc argc as passed to main().
        MPIComms(int argc, char *argv[]);

        /// @brief Destructor
        virtual ~MPIComms();

        /// Returns the name of the node (i.e. the hostname)
        virtual std::string nodeName(void) const;

        /// @brief Returns the MPI rank of the calling process
        virtual int rank(void) const;

        /// @brief Returns the size of the communicator group (i.e. the number
        /// of processes)
        virtual int nProcs(void) const;

        /// @brief Request all nodes in the communictor group abort.
        virtual void abort(void);

        /// @brief MPI_Send a raw buffer to the specified destination
        /// process.
        ///
        /// @param[in] buf a pointer to the buffer to send.
        /// @param[in] size    the number of bytes to send.
        /// @param[in] dest    the id of the process to send to.
        /// @param[in] tag the MPI tag to be used in the communication.
        virtual void send(const void* buf, size_t size, int dest, int tag = 0);

        /// @brief MPI_Recv a raw buffer from the specified source process.
        ///
        /// @param[out] buf a pointer to the buffer to receive data into.
        /// @param[in] size    the number of bytes to receive.
        /// @param[in] source  the id of the process to receive from.
        /// @param[in] tag the MPI tag to be used in the communication.
        virtual void receive(void* buf, size_t size, int source, int tag = 0);

        /// @brief MPI_Recv a raw buffer from any source process.
        ///
        /// @param[out] buf a pointer to the buffer to receive data into.
        /// @param[in] size    the number of bytes to receive.
        /// @param[in] tag the MPI tag to be used in the communication.
        /// @return the MPI rank of the process from which the message
        ///         was received.
        virtual int receiveAnySrc(void* buf, size_t size, int tag = 0);

        /// @brief MPI_Bcast a raw buffer.
        ///
        /// @param[in,out] buf    data buffer.
        /// @param[in] size       number of bytes to broadcast.
        /// @param[in] root       id of the root process.
        virtual void broadcast(void* buf, size_t size, int root);

    private:
        // Check for error status and handle accordingly
        void checkError(const int error, const std::string location) const;

        // Add a byte offset to the  specified pointer, returning the result
        void* addOffset(const void *ptr, size_t offset) const;

#ifdef HAVE_MPI
        // Specific MPI Communicator for this class
        MPI_Comm itsCommunicator;
#endif

        // No support for assignment
        MPIComms& operator=(const MPIComms& rhs);

        // No support for copy constructor
        MPIComms(const MPIComms& src);
};

}
}

#endif
