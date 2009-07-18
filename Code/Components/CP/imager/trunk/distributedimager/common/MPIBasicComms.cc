/// @file MPIBasicComms.cc
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

// Include own header file first
#include "MPIBasicComms.h"

// System includes
#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <stdint.h>

// MPI includes
#include <mpi.h>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/OS/Timer.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufVector.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobOBufVector.h>

// Local package includes
#include <messages/IMessage.h>


using namespace askap::cp;

ASKAP_LOGGER(logger, ".MPIBasicComms");

MPIBasicComms::MPIBasicComms(int argc, char *argv[])
{
    int rc = MPI_Init(&argc, &argv);

    if (rc != MPI_SUCCESS) {
        ASKAPTHROW (std::runtime_error, "Error starting MPI. Terminating.");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    // Duplicate the communicator so this class
    // doesn't conflict with other uses of MPI
    MPI_Comm_dup(MPI_COMM_WORLD, &itsCommunicator);

    // To aid in debugging, now we know the MPI rank
    // set the ID in the logger
    int rank;
    MPI_Comm_rank(itsCommunicator, &rank);
    std::ostringstream ss;
    ss << rank;
    ASKAPLOG_REMOVECONTEXT("mpirank");
    ASKAPLOG_PUTCONTEXT("mpirank", ss.str().c_str());

    // Also set the nodename
    char name[MPI_MAX_PROCESSOR_NAME];
    int resultlen;
    MPI_Get_processor_name(name, &resultlen);
    std::string pname(name);
    std::string::size_type idx = pname.find_first_of('.');
    if (idx != std::string::npos) {
        // Extract just the hostname part
        pname = pname.substr(0, idx);
    }
    ASKAPLOG_REMOVECONTEXT("hostname");
    ASKAPLOG_PUTCONTEXT("hostname", pname.c_str());
}

MPIBasicComms::~MPIBasicComms()
{
    MPI_Comm_free(&itsCommunicator);
    MPI_Finalize();
}

int MPIBasicComms::getId(void)
{
    int rank = -1;
    int result = MPI_Comm_rank(itsCommunicator, &rank);
    checkError(result, "MPI_Comm_rank");

    return rank;
}

int MPIBasicComms::getNumNodes(void)
{
    int numtasks = -1;
    int result = MPI_Comm_size(itsCommunicator, &numtasks);
    checkError(result, "MPI_Comm_size");

    return numtasks;
}

void MPIBasicComms::abort(void)
{
    int result = MPI_Abort(itsCommunicator, 0);
    checkError(result, "MPI_Abort");
}

void MPIBasicComms::send(const void* buf, size_t size, int dest, int tag)
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
        if (remaining > c_maxint) {
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

    ASKAPCHECK(remaining == 0, "MPIBasicComms::send() Didn't send all data");
}

void MPIBasicComms::receive(void* buf, size_t size, int source, int tag, MPI_Status& status)
{
    const unsigned int c_maxint = std::numeric_limits<int>::max();

    // First receive the size of the payload to be received,
    // remembering the size parameter passed to this function is
    // just the maximum size of the buffer, and hence the maximum
    // number of bytes that can be received.
    unsigned long payloadSize;
    int result = MPI_Recv(&payloadSize, 1, MPI_UNSIGNED_LONG,
            source, tag, itsCommunicator, &status);
    checkError(result, "MPI_Recv");

    // The source parameter may be MPI_ANY_SOURCE, so the actual
    // source needs to be recorded for later use.
    const int actualSource = status.MPI_SOURCE;

    // Receive the smaller of size or payloadSize
    size_t remaining = (payloadSize > size) ? size : payloadSize;

    while (remaining > 0) {
        size_t offset = size - remaining;
        void* addr = addOffset(buf, offset);
        if (remaining > c_maxint) {
            result = MPI_Recv(addr, c_maxint, MPI_BYTE,
                    actualSource, tag, itsCommunicator, &status);
            remaining -= c_maxint;
        } else {
            result = MPI_Recv(addr, remaining, MPI_BYTE,
                    actualSource, tag, itsCommunicator, &status);
            remaining = 0;
        }
        checkError(result, "MPI_Recv");
    }

    ASKAPCHECK(remaining == 0, "MPIBasicComms::receive() Didn't receive all data");
}

void MPIBasicComms::broadcast(void* buf, size_t size, int root)
{
    const unsigned int c_maxint = std::numeric_limits<int>::max();

    // Broadcast in chunks of size MAXINT until complete
    size_t remaining = size;
    int result;
    while (remaining > 0) {
        size_t offset = size - remaining;

        void* addr = addOffset(buf, offset);
        if (remaining > c_maxint) {
            result = MPI_Bcast(addr, c_maxint, MPI_BYTE, root, itsCommunicator);
            remaining -= c_maxint;
        } else {
            result = MPI_Bcast(addr, remaining, MPI_BYTE, root, itsCommunicator);
            remaining = 0;
        }
        checkError(result, "MPI_Bcast");
    }
}

void MPIBasicComms::checkError(const int error, const std::string location)
{
    if (error == MPI_SUCCESS) {
        return;
    }

    char estring[MPI_MAX_ERROR_STRING];
    int eclass;
    int len;

    MPI_Error_class(error, &eclass);
    MPI_Error_string(error, estring, &len);
    ASKAPTHROW (std::runtime_error, "" << location << " failed. Error  "
            << eclass << ": " << estring);
}

void* MPIBasicComms::addOffset(const void *ptr, size_t offset)
{
    char *cptr = static_cast<char*>(const_cast<void*>(ptr));
    cptr += offset;

    return cptr;
}

void MPIBasicComms::sendMessage(const IMessage& msg, int dest)
{
    // Encode
    std::vector<int8_t> buf;
    LOFAR::BlobOBufVector<int8_t> bv(buf);
    LOFAR::BlobOStream out(bv);
    out.putStart("Message", 1);
    out << msg;
    out.putEnd();

    int messageType = msg.getMessageType();

    casa::Timer timer;
    timer.mark();

    // First send the size of the buffer
    const long size = buf.size();
    send(&size, sizeof(long), dest, messageType);

    // Now send the actual byte stream
    send(&buf[0], size * sizeof(int8_t), dest, messageType);

    ASKAPLOG_INFO_STR(logger, "Sent Message of type " << messageType
            << " to rank " << dest << " via MPI in " << timer.real()
            << " seconds ");
}

void MPIBasicComms::receiveMessage(IMessage& msg, int source)
{
    // First receive the size of the byte stream
    long size;
    const int type = msg.getMessageType();
    MPI_Status status;  // Not really used
    receive(&size, sizeof(long), source, type, status);

    // Receive the byte stream
    std::vector<int8_t> buf;
    buf.resize(size);
    receive(&buf[0], size * sizeof(char), source, type, status);

    // Decode
    LOFAR::BlobIBufVector<int8_t> bv(buf);
    LOFAR::BlobIStream in(bv);
    int version = in.getStart("Message");
    ASKAPASSERT(version == 1);
    in >> msg;
    in.getEnd();
}

void MPIBasicComms::receiveMessageAnySrc(IMessage& msg, int& actualSource)
{
    // First receive the size of the byte stream
    long size;
    const int type = msg.getMessageType();
    MPI_Status status;
    receive(&size, sizeof(long), MPI_ANY_SOURCE, type, status);
    actualSource = status.MPI_SOURCE;

    // Receive the byte stream
    std::vector<int8_t> buf;
    buf.resize(size);
    receive(&buf[0], size * sizeof(char), actualSource, type, status);

    // Decode
    LOFAR::BlobIBufVector<int8_t> bv(buf);
    LOFAR::BlobIStream in(bv);
    int version = in.getStart("Message");
    ASKAPASSERT(version == 1);
    in >> msg;
    in.getEnd();
}

void MPIBasicComms::receiveMessageAnySrc(IMessage& msg)
{
    int id;
    receiveMessageAnySrc(msg, id); 
    return;
}

void MPIBasicComms::sendMessageBroadcast(const IMessage& msg)
{
    std::vector<int8_t> buf;
    int root = getId();

    // Encode the model to a byte stream
    LOFAR::BlobOBufVector<int8_t> bv(buf);
    LOFAR::BlobOStream out(bv);
    out.putStart("BroadcastMessage", 1);
    out << msg;
    out.putEnd();

    casa::Timer timer;
    timer.mark();

    // First broadcast the size of the mesage broadcast
    unsigned long size = buf.size();
    broadcast(&size, sizeof(unsigned long), root);

    // Now broadcast the message itself
    broadcast(&buf[0], size * sizeof(int8_t), root);

    ASKAPLOG_INFO_STR(logger, "Broadcast model to all ranks via MPI in "
            << timer.real() << " seconds ");

}

void MPIBasicComms::receiveMessageBroadcast(IMessage& msg, int root)
{
    // Participate in the size broadcast
    unsigned long size;
    broadcast(&size, sizeof(unsigned long), root);

    // Setup a data buffer to receive into
    std::vector<int8_t> buf;
    buf.resize(size);

    // Now participate in the broadcast of the message itself
    broadcast(&buf[0], size * sizeof(int8_t), root);

    // Decode
    LOFAR::BlobIBufVector<int8_t> bv(buf);
    LOFAR::BlobIStream in(bv);
    int version = in.getStart("BroadcastMessage");
    ASKAPASSERT(version == 1);
    in >> msg;
    in.getEnd();
}

