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

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <stdint.h>
#include <limits>

// MPI includes
#include <mpi.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/OS/Timer.h"
#include "skymodelclient/Component.h"

using namespace askap::cp::pipelinetasks;

ASKAP_LOGGER(logger, ".MPIBasicComms");

MPIBasicComms::MPIBasicComms(int argc, char *argv[]) :
    itsComponentTag(1), itsReadyTag(2)
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
        if (remaining >= c_maxint) {
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

void MPIBasicComms::broadcastParset(LOFAR::ParameterSet& parset, int root)
{
    // First broadcast the size of parset (number of elements)
    int size = parset.size();
    broadcast(&size, sizeof(int), root);

    int keyLen = -1;
    int valLen = -1;
    if (getId() == 0) {
        int count = 0;
        LOFAR::ParameterSet::const_iterator it;
        for (it = parset.begin(); it != parset.end(); ++it) {
            count++;
            // Broadcast the key length then value length
            const std::string key = it->first;
            const std::string val = it->second;
            keyLen = key.length();
            valLen = val.length();
            broadcast(&keyLen, sizeof(int), root);
            broadcast(&valLen, sizeof(int), root);

            broadcast(const_cast<char*>(key.c_str()), sizeof(char) * key.length(), root);
            broadcast(const_cast<char*>(val.c_str()), sizeof(char) * val.length(), root);
        }
        ASKAPCHECK(count == size, "Iterator returned fewer/more elements than expected");
    } else {
        for (int i = 0; i < size; ++i) {
            broadcast(&keyLen, sizeof(int), root);
            broadcast(&valLen, sizeof(int), root);
            char key[keyLen+1];
            memset(key, 0, keyLen+1);
            char val[valLen+1];
            memset(val, 0, valLen+1);
            broadcast(key, sizeof(char) * keyLen, root);
            broadcast(val, sizeof(char) * valLen, root);
            parset.add(key, val);
        }
    }
}

void MPIBasicComms::sendComponents(const std::vector<askap::cp::skymodelservice::Component>& components, int dest)
{
    // First send the number of elements
    int size = components.size();
    send(&size, sizeof(int), dest, itsComponentTag);

    const int nDoubles = 8;
    double payload[nDoubles];

    for (int i = 0; i < size; ++i) {
        // Extract the data from the component
        long id = components[i].id();
        payload[0] = components[i].rightAscension().getValue("deg");
        payload[1] = components[i].declination().getValue("deg");
        payload[2] = components[i].positionAngle().getValue("rad");
        payload[3] = components[i].majorAxis().getValue("arcsec");
        payload[4] = components[i].minorAxis().getValue("arcsec");
        payload[5] = components[i].i1400().getValue("Jy");
        payload[6] = components[i].spectralIndex();
        payload[7] = components[i].spectralCurvature();

        send(&id, sizeof(long), dest, itsComponentTag);
        send(&payload, sizeof(double) * nDoubles, dest, itsComponentTag);
    }
}

std::vector<askap::cp::skymodelservice::Component> MPIBasicComms::receiveComponents(int source)
{
    MPI_Status status; // not really used

    // First receive the number of elements to expect
    int size;
    receive(&size, sizeof(int), source, itsComponentTag, status);

    const int nDoubles = 8;
    double payload[nDoubles];

    std::vector<askap::cp::skymodelservice::Component> components;
    if (size > 0) {
        components.reserve(size);
    }
    for (int i = 0; i < size; ++i) {
        long id;
        receive(&id, sizeof(long), source, itsComponentTag, status);
        receive(&payload, sizeof(double) * nDoubles, source, itsComponentTag, status);

        components.push_back(askap::cp::skymodelservice::Component(id,
                    casa::Quantity(payload[0], "deg"),
                    casa::Quantity(payload[1], "deg"),
                    casa::Quantity(payload[2], "rad"),
                    casa::Quantity(payload[3], "arcsec"),
                    casa::Quantity(payload[4], "arcsec"),
                    casa::Quantity(payload[5], "Jy"),
                    payload[6],
                    payload[7])); 
    }

    return components;
}

void MPIBasicComms::sumImages(casa::ImageInterface<casa::Float>& image, int root)
{
    casa::Array<casa::Float> src = image.get();
    if (getId() == 0) {
        casa::Array<casa::Float> dest = image.get();
        MPI_Reduce(src.data(), dest.data(), src.size(), MPI_FLOAT, MPI_SUM, root, itsCommunicator);
        image.put(dest);
    } else {
        MPI_Reduce(src.data(), 0, src.size(), MPI_FLOAT, MPI_SUM, root, itsCommunicator);
    }
}

void MPIBasicComms::signalReady(int dest)
{
    // Note, there is nothing significant about sending the id,
    // this message is interpreted as a "ready" signal on account
    // of the tag used. Although it is verified in the method
    // getReadyWorkerId() as a simple consisteny check.
    int id = getId();
    send(&id, sizeof(int), dest, itsReadyTag);
}

int MPIBasicComms::getReadyWorkerId(void)
{
    MPI_Status status;
    int id;
    receive(&id, sizeof(int), MPI_ANY_SOURCE, itsReadyTag, status);
    ASKAPCHECK(id == status.MPI_SOURCE, "Expected payload to equal MPI_SOURCE");
    return status.MPI_SOURCE;
}
