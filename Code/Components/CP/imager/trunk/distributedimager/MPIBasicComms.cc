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
#include <messages/MessageFactory.h>


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

void MPIBasicComms::sendString(const std::string& str, int dest)
{
    // First send the size of the string
    int size = str.size() + 1;
    send(&size, sizeof(int), dest, STRING);

    // Now send the actual string
    send(str.c_str(), size * sizeof(char), dest, STRING);
}

std::string MPIBasicComms::receiveString(int source)
{
    MPI_Status status;  // Not used but needed to call receive()

    // First receive the size of the string
    int size;
    receive(&size, sizeof(int), source, STRING, status);

    // Allocate a recv buffer then recv
    char buf[size];
    memset(buf, 0, size);
    receive(buf, size * sizeof(char), source, STRING, status);

    return std::string(buf);
}

std::string MPIBasicComms::receiveStringAny(int& source)
{
    MPI_Status status;  // Not used but needed to call receive()

    // First receive the size of the string
    int size;
    receive(&size, sizeof(int), MPI_ANY_SOURCE, STRING, status);

    int actualSource = status.MPI_SOURCE;

    // Allocate a recv buffer then recv
    char buf[size];
    memset(buf, 0, size);
    receive(buf, size * sizeof(char), actualSource, STRING, status);

    source = actualSource;

    return std::string(buf);
}

void MPIBasicComms::send(const void* buf, size_t size, int dest, int tag)
{
    unsigned int c_maxint = std::numeric_limits<int>::max();

    int result;

    // First send the size of the buffer.
    unsigned long lsize = size;  // Promote for simplicity
    result = MPI_Send(&lsize, 1, MPI_UNSIGNED_LONG, dest, tag, itsCommunicator);
    checkError(result, "MPI_Send");

    // Send in chunks of size MAXINT until complete
    size_t remaining = size;
    while (remaining > 0) {
        int result;
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
    unsigned int c_maxint = std::numeric_limits<int>::max();

    int result;

    // First receive the size of the payload to be received,
    // remembering the size parameter passed to this function is
    // just the maximum size of the buffer, and hence the maximum
    // number of bytes that can be received.
    unsigned long payloadSize;
    result = MPI_Recv(&payloadSize, 1, MPI_UNSIGNED_LONG,
            source, tag, itsCommunicator, &status);
    checkError(result, "MPI_Recv");

    // The source parameter may be MPI_ANY_SOURCE, so the actual
    // source needs to be recorded for later use.
    int actualSource = status.MPI_SOURCE;

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
    int result = MPI_Bcast(buf, size, MPI_BYTE, root, itsCommunicator);
    checkError(result, "MPI_Bcast");
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
    long size = buf.size();
    send(&size, sizeof(long), dest, messageType);

    // Now send the actual byte stream
    send(&buf[0], size * sizeof(int8_t), dest, messageType);

    ASKAPLOG_INFO_STR(logger, "Sent Message of type " << messageType
            << " to rank " << dest << " via MPI in " << timer.real()
            << " seconds ");
}

const IMessageSharedPtr MPIBasicComms::receiveMessage(IMessage::MessageType type, int source)
{
    // First receive the size of the byte stream
    long size;
    MPI_Status status;  // Not really used
    receive(&size, sizeof(long), source, type, status);

    // Receive the byte stream
    std::vector<int8_t> buf;
    buf.resize(size);
    receive(&buf[0], size * sizeof(char), source, type, status);

    // Create a message to populate
    MessageFactory factory;
    IMessageSharedPtr msg(factory.create(type));

    // Decode
    LOFAR::BlobIBufVector<int8_t> bv(buf);
    LOFAR::BlobIStream in(bv);
    int version = in.getStart("Message");
    ASKAPASSERT(version == 1);
    in >> *msg;
    in.getEnd();

    return msg;
}

const IMessageSharedPtr MPIBasicComms::receiveMessageAnySrc(IMessage::MessageType type, int& actualSource)
{
    // First receive the size of the byte stream
    long size;
    MPI_Status status;
    receive(&size, sizeof(long), MPI_ANY_SOURCE, type, status);
    actualSource = status.MPI_SOURCE;

    // Receive the byte stream
    std::vector<int8_t> buf;
    buf.resize(size);
    receive(&buf[0], size * sizeof(char), actualSource, type, status);

    // Create a message to populate
    MessageFactory factory;
    IMessageSharedPtr msg(factory.create(type));

    // Decode
    LOFAR::BlobIBufVector<int8_t> bv(buf);
    LOFAR::BlobIStream in(bv);
    int version = in.getStart("Message");
    ASKAPASSERT(version == 1);
    in >> *msg;
    in.getEnd();

    return msg;
}

