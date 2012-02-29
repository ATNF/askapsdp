/// @file MPIComms.cc
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

// Include own header file first
#include "MPIComms.h"

// Include package level header file
#include "askap_askapparallel.h"

// System includes
#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <limits>

// MPI includes
#ifdef HAVE_MPI
#include <mpi.h>
#endif

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"

using namespace askap::askapparallel;

ASKAP_LOGGER(logger, ".MPIComms");

#ifdef HAVE_MPI
MPIComms::MPIComms(int argc, char *argv[])
{
    int rc = MPI_Init(&argc, &argv);

    if (rc != MPI_SUCCESS) {
        ASKAPTHROW(AskapError, "Error starting MPI. Terminating.");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    // Duplicate the communicator so this class
    // doesn't conflict with other uses of MPI
    MPI_Comm_dup(MPI_COMM_WORLD, &itsCommunicator);
}

MPIComms::~MPIComms()
{
    MPI_Comm_free(&itsCommunicator);
    MPI_Finalize();
}

std::string MPIComms::nodeName(void) const
{
    char name[MPI_MAX_PROCESSOR_NAME];
    int resultlen;
    MPI_Get_processor_name(name, &resultlen);
    std::string pname(name);
    std::string::size_type idx = pname.find_first_of('.');

    if (idx != std::string::npos) {
        // Extract just the hostname part
        pname = pname.substr(0, idx);
    }

    return pname;
}

int MPIComms::rank(void) const
{
    int rank = -1;
    int result = MPI_Comm_rank(itsCommunicator, &rank);
    checkError(result, "MPI_Comm_rank");

    return rank;
}

int MPIComms::nProcs(void) const
{
    int numtasks = -1;
    int result = MPI_Comm_size(itsCommunicator, &numtasks);
    checkError(result, "MPI_Comm_size");

    return numtasks;
}

void MPIComms::abort(void)
{
    int result = MPI_Abort(itsCommunicator, 0);
    checkError(result, "MPI_Abort");
}

void MPIComms::send(const void* buf, size_t size, int dest, int tag)
{
    const unsigned int c_maxint = std::numeric_limits<int>::max();

    // First send the size of the buffer.
    unsigned long lsize = size;  // Promote for simplicity
    int result = MPI_Send(&lsize, 1, MPI_UNSIGNED_LONG, dest, tag, itsCommunicator);
    checkError(result, "MPI_Send");

    // Send in chunks of size MAXINT until complete
    size_t remaining = size;

    while (remaining > 0) {
        size_t offset = size - remaining;

        void* addr = addOffset(buf, offset);

        if (remaining >= c_maxint) {
            result = MPI_Send(addr, c_maxint, MPI_BYTE,
                              dest, tag, itsCommunicator);
            remaining -= c_maxint;
        } else {
            result = MPI_Send(addr, remaining, MPI_BYTE,
                              dest, tag, itsCommunicator);
            remaining = 0;
        }

        checkError(result, "MPI_Send");
    }

    ASKAPCHECK(remaining == 0, "MPIComms::send() Didn't send all data");
}

void MPIComms::receive(void* buf, size_t size, int source, int tag)
{
    receiveImpl(buf, size, source, tag);
}

int MPIComms::receiveAnySrc(void* buf, size_t size, int tag)
{
    return receiveImpl(buf, size, MPI_ANY_SOURCE, tag);
}

int MPIComms::receiveImpl(void* buf, size_t size, int source, int tag)
{
    const unsigned int c_maxint = std::numeric_limits<int>::max();

    // First receive the size of the payload to be received,
    // remembering the size parameter passed to this function is
    // just the maximum size of the buffer, and hence the maximum
    // number of bytes that can be received.
    unsigned long payloadSize;
    MPI_Status status;
    int result = MPI_Recv(&payloadSize, 1, MPI_UNSIGNED_LONG,
                          source, tag, itsCommunicator, &status);
    checkError(result, "MPI_Recv");

    // The source parameter may be MPI_ANY_SOURCE, so the actual
    // source needs to be recorded for later use.
    const int actualSource = status.MPI_SOURCE;
    if (source != MPI_ANY_SOURCE) {
        ASKAPCHECK(actualSource == source,
                "Actual source of message differs from requested source");
    }

    // Receive the smaller of size or payloadSize
    size_t remaining = (payloadSize > size) ? size : payloadSize;

    while (remaining > 0) {
        size_t offset = size - remaining;
        void* addr = addOffset(buf, offset);

        if (remaining >= c_maxint) {
            result = MPI_Recv(addr, c_maxint, MPI_BYTE,
                              actualSource, tag, itsCommunicator, MPI_STATUS_IGNORE);
            remaining -= c_maxint;
        } else {
            result = MPI_Recv(addr, remaining, MPI_BYTE,
                              actualSource, tag, itsCommunicator, MPI_STATUS_IGNORE);
            remaining = 0;
        }

        checkError(result, "MPI_Recv");
    }

    ASKAPCHECK(remaining == 0, "MPIComms::receive() Didn't receive all data");
    return actualSource;
}

void MPIComms::broadcast(void* buf, size_t size, int root)
{
    const unsigned int c_maxint = std::numeric_limits<int>::max();

    // Broadcast in chunks of size MAXINT until complete
    size_t remaining = size;
    int result;

    while (remaining > 0) {
        size_t offset = size - remaining;

        void* addr = addOffset(buf, offset);

        if (remaining >= c_maxint) {
            result = MPI_Bcast(addr, c_maxint, MPI_BYTE, root, itsCommunicator);
            remaining -= c_maxint;
        } else {
            result = MPI_Bcast(addr, remaining, MPI_BYTE, root, itsCommunicator);
            remaining = 0;
        }

        checkError(result, "MPI_Bcast");
    }
}

void MPIComms::checkError(const int error, const std::string location) const
{
    if (error == MPI_SUCCESS) {
        return;
    }

    char estring[MPI_MAX_ERROR_STRING];
    int eclass;
    int len;

    MPI_Error_class(error, &eclass);
    MPI_Error_string(error, estring, &len);
    ASKAPTHROW(std::runtime_error, "" << location << " failed. Error  "
                   << eclass << ": " << estring);
}

#else

MPIComms::MPIComms(int argc, char *argv[])
{
}

MPIComms::~MPIComms()
{
}

std::string MPIComms::nodeName(void) const
{
    const int MAX_HOSTNAME_LEN = 1024;
    char name[MAX_HOSTNAME_LEN];
    name[MAX_HOSTNAME_LEN - 1] = '\0';
    const int error = gethostname(name, MAX_HOSTNAME_LEN - 1);
    if (error) {
        ASKAPTHROW(AskapError, "MPIComms::nodeName() returned error: " << error);
    }

    std::string pname(name);
    std::string::size_type idx = pname.find_first_of('.');
    if (idx != std::string::npos) {
        // Extract just the hostname part
        pname = pname.substr(0, idx);
    }

    return pname;
}

int MPIComms::rank(void) const
{
    return 0;
}

int MPIComms::nProcs(void) const
{
    return 1;
}

void MPIComms::abort(void)
{
    exit(1);
}

void MPIComms::send(const void* buf, size_t size, int dest, int tag)
{
    ASKAPTHROW(AskapError, "MPIComms::send() cannot be used - configured without MPI");
}

void MPIComms::receive(void* buf, size_t size, int source, int tag)
{
    ASKAPTHROW(AskapError, "MPIComms::receive() cannot be used - configured without MPI");
}

int MPIComms::receiveAnySrc(void* buf, size_t size, int tag)
{
    ASKAPTHROW(AskapError, "MPIComms::receiveAnySrc() cannot be used - configured without MPI");
}

void MPIComms::broadcast(void* buf, size_t size, int root)
{
    ASKAPTHROW(AskapError, "MPIComms::broadcast() cannot be used - configured without MPI");
}

void MPIComms::checkError(const int error, const std::string location) const
{
    ASKAPTHROW(AskapError, "MPIComms::checkError() cannot be used - configured without MPI");
}

#endif

//////////////////////////////////////////////////////////////////////////
// Shared methods (i.e. by both MPI and non-mpi implementations go below
//////////////////////////////////////////////////////////////////////////

void* MPIComms::addOffset(const void *ptr, size_t offset) const
{
    char *cptr = static_cast<char*>(const_cast<void*>(ptr));
    cptr += offset;

    return cptr;
}
