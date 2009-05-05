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
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/Params.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufVector.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobOBufVector.h>
#include <Blob/BlobArray.h>
#include <casa/OS/Timer.h>
#include <casa/Arrays/Array.h>

using namespace askap::cp;
using namespace askap::scimath;

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
    MPI_Comm_dup(MPI_COMM_WORLD, &m_communicator);

    // To aid in debugging, now we know the MPI rank
    // set the ID in the logger
    int rank;
    MPI_Comm_rank(m_communicator, &rank);
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
    if (idx != string::npos) {
        // Extract just the hostname part
        pname = pname.substr(0, idx);
    }
    ASKAPLOG_REMOVECONTEXT("hostname");
    ASKAPLOG_PUTCONTEXT("hostname", pname.c_str());
}

MPIBasicComms::~MPIBasicComms()
{
    MPI_Comm_free(&m_communicator);
    MPI_Finalize();
}

int MPIBasicComms::getId(void)
{
    int rank = -1;
    int result = MPI_Comm_rank(m_communicator, &rank);
    handleError(result, "MPI_Comm_rank");

    return rank;
}

int MPIBasicComms::getNumNodes(void)
{
    int numtasks = -1;
    int result = MPI_Comm_size(m_communicator, &numtasks);
    handleError(result, "MPI_Comm_size");

    return numtasks;
}

void MPIBasicComms::abort(void)
{
    int result = MPI_Abort(m_communicator, 0);
    handleError(result, "MPI_Abort");
}

void MPIBasicComms::broadcastModel(askap::scimath::Params::ShPtr model)
{
    casa::Timer timer;
    timer.mark();

    // Encode the model to a byte stream
    std::vector<char> data;
    LOFAR::BlobOBufVector<char> bv(data);
    LOFAR::BlobOStream out(bv);
    out.putStart("model", 1);
    out << *model;
    out.putEnd();

    // First broadcast the size of the model broadcast
    unsigned long size = data.size();
    broadcast(&size, sizeof(unsigned long), c_root);

    // Now broadcast the model itself
    broadcast(&data[0], data.size() * sizeof(char), c_root);

    ASKAPLOG_INFO_STR(logger, "Broadcast model to all ranks via MPI in "
            << timer.real() << " seconds ");
}

askap::scimath::Params::ShPtr MPIBasicComms::receiveModel(void)
{
    // Participate in the broadcast to receive the size of the model
    unsigned long size;
    broadcast(&size, sizeof(unsigned long), c_root);

    // Participate in the broadcast to receive the model
    std::vector<char> data;
    data.resize(size);
    broadcast(&data[0], data.size() * sizeof(char), c_root);

    // Decode
    Params::ShPtr model_p = Params::ShPtr(new Params());
    LOFAR::BlobIBufVector<char> bv(data);
    LOFAR::BlobIStream in(bv);
    int version = in.getStart("model");
    if (version != 1) {
        ASKAPTHROW (std::runtime_error, "Model byte stream is of incorrect version");
    }
    in >> *model_p;
    in.getEnd();

    return model_p;
}

void MPIBasicComms::sendNE(askap::scimath::INormalEquations::ShPtr ne_p, int id, int count)
{
    casa::Timer timer;
    timer.mark();

    // Encode the normal equations to a byte stream
    std::vector<char> data;
    LOFAR::BlobOBufVector<char> bv(data);
    LOFAR::BlobOStream out(bv);
    out.putStart("ne", 1);
    out << count;
    out << *ne_p;
    out.putEnd();


    // First send the size of the model
    long size = data.size();
    ASKAPLOG_INFO_STR(logger, "Sending normal equations of size " << size
            << " to id " << id);
    send(&size, sizeof(long), id, NORMAL_EQUATION);

    // Now send the actual byte stream
    send(&data[0], size * sizeof(char), id, NORMAL_EQUATION);

    ASKAPLOG_INFO_STR(logger, "Sent NormalEquations to rank " << id
            << " via MPI in " << timer.real() << " seconds ");
}

askap::scimath::INormalEquations::ShPtr MPIBasicComms::receiveNE(int& id, int& count)
{
    // First receive the size of the byte stream
    long size;
    MPI_Status status;
    receive(&size, sizeof(long), MPI_ANY_SOURCE, NORMAL_EQUATION, status);
    ASKAPLOG_INFO_STR(logger, "About to recv normal equations of size " << size);

    // Receive the byte stream
    std::vector<char> data;
    data.resize(size);
    receive(&data[0], size * sizeof(char), status.MPI_SOURCE, NORMAL_EQUATION, status);

    // Decode
    askap::scimath::INormalEquations::ShPtr ne_p = ImagingNormalEquations::ShPtr(new ImagingNormalEquations());
    LOFAR::BlobIBufVector<char> bv(data);
    LOFAR::BlobIStream in(bv);
    int version = in.getStart("ne");
    if (version != 1) {
        ASKAPTHROW (std::runtime_error, "Normal Equations byte stream is of incorrect version");
    }
    in >> count;
    in >> *ne_p;
    in.getEnd();

    id = status.MPI_SOURCE;

    return ne_p;
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
    result = MPI_Send(&lsize, 1, MPI_UNSIGNED_LONG, dest, tag, m_communicator);
    handleError(result, "MPI_Send");

    // Send in chunks of size MAXINT until complete
    size_t remaining = size;
    while (remaining > 0) {
        int result;
        size_t offset = size - remaining;

        void* addr = addOffset(buf, offset);
        if (remaining > c_maxint) {
            result = MPI_Send(addr, c_maxint, MPI_BYTE,
                    dest, tag, m_communicator);
            remaining -= c_maxint;
        } else {
            result = MPI_Send(addr, remaining, MPI_BYTE,
                    dest, tag, m_communicator);
            remaining = 0;
        }
        handleError(result, "MPI_Send");
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
            source, tag, m_communicator, &status);
    handleError(result, "MPI_Recv");

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
                    actualSource, tag, m_communicator, &status);
            remaining -= c_maxint;
        } else {
            result = MPI_Recv(addr, remaining, MPI_BYTE,
                    actualSource, tag, m_communicator, &status);
            remaining = 0;
        }
        handleError(result, "MPI_Recv");
    }

    ASKAPCHECK(remaining == 0, "MPIBasicComms::receive() Didn't receive all data");
}

void MPIBasicComms::broadcast(void* buf, size_t size, int root)
{
    int result = MPI_Bcast(buf, size, MPI_BYTE, root, m_communicator);
    handleError(result, "MPI_Bcast");
}

void MPIBasicComms::handleError(const int error, const std::string location)
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

void MPIBasicComms::sendCleanRequest(int patchid,
        const casa::Array<float>& dirty,
        const casa::Array<float>& psf,
        const casa::Array<float>& mask,
        const casa::Array<float>& model,
        double threshold,
        std::string thresholdUnits,
        double fractionalThreshold,
        std::vector<float>& scales,
        int niter,
        double gain,
        int dest)
{
    casa::Timer timer;
    timer.mark();

    // Encode
    std::vector<int8_t> buf;
    LOFAR::BlobOBufVector<int8_t> bv(buf);
    LOFAR::BlobOStream out(bv);
    out.putStart("cleanrequest", 1);
    out << patchid;
    out << dirty;
    out << psf;
    out << mask;
    out << model;
    out << threshold;
    out << thresholdUnits;
    out << fractionalThreshold;
    out << scales;
    out << niter;
    out << gain;
    out.putEnd();

    // First send the size of the buffer
    long size = buf.size();
    ASKAPLOG_INFO_STR(logger, "Sending size of " << size);
    send(&size, sizeof(long), dest, CLEAN_REQUEST);

    // Now send the actual byte stream
    ASKAPLOG_INFO_STR(logger, "Now sending the actual buffer");
    send(&buf[0], size * sizeof(int8_t), dest, CLEAN_REQUEST);

    ASKAPLOG_INFO_STR(logger, "Sent CleanRequest to rank " << dest
            << " via MPI in " << timer.real() << " seconds ");
}

void MPIBasicComms::recvCleanRequest(int& patchid,
        casa::Array<float>& dirty,
        casa::Array<float>& psf,
        casa::Array<float>& mask,
        casa::Array<float>& model,
        double& threshold,
        std::string& thresholdUnits,
        double& fractionalThreshold,
        std::vector<float>& scales,
        int& niter,
        double& gain)
{
    ASKAPLOG_INFO_STR(logger, "Waiting for the size...");
    // First receive the size of the byte stream
    long size;
    MPI_Status status;
    receive(&size, sizeof(long), MPI_ANY_SOURCE, CLEAN_REQUEST, status);
    ASKAPLOG_INFO_STR(logger, "Preparing to recv size of " << size);

    // Receive the byte stream
    std::vector<int8_t> buf;
    buf.resize(size);
    ASKAPLOG_INFO_STR(logger, "Resizd buffer");
    receive(&buf[0], size * sizeof(int8_t), status.MPI_SOURCE, CLEAN_REQUEST, status);
    ASKAPLOG_INFO_STR(logger, "Recv complete");

    // Decode
    LOFAR::BlobIBufVector<int8_t> bv(buf);
    LOFAR::BlobIStream in(bv);
    int version = in.getStart("cleanrequest");
    ASKAPASSERT(version == 1);
    in >> patchid;
    in >> dirty;
    in >> psf;
    in >> mask;
    in >> model;
    in >> threshold;
    in >> thresholdUnits;
    in >> fractionalThreshold;
    in >> scales;
    in >> niter;
    in >> gain;
    in.getEnd();
}

void MPIBasicComms::sendCleanResponse(int patchid,
        casa::Array<float>& patch,
        double strengthOptimum,
        int dest)
{
    casa::Timer timer;
    timer.mark();

    // Encode
    std::vector<int8_t> buf;
    LOFAR::BlobOBufVector<int8_t> bv(buf);
    LOFAR::BlobOStream out(bv);
    out.putStart("cleanresponse", 1);
    out << patchid;
    out << patch;
    out << strengthOptimum;
    out.putEnd();

    // First send the size of the buffer
    long size = buf.size();
    send(&size, sizeof(long), dest, CLEAN_RESPONSE);

    // Now send the actual byte stream
    send(&buf[0], size * sizeof(int8_t), dest, CLEAN_RESPONSE);

    ASKAPLOG_INFO_STR(logger, "Sent CleanResponse to rank " << dest
            << " via MPI in " << timer.real() << " seconds ");
}

void MPIBasicComms::recvCleanResponse(int& patchid,
        casa::Array<float>& patch,
        double& strengthOptimum)
{
    // First receive the size of the byte stream
    long size;
    MPI_Status status;
    receive(&size, sizeof(long), MPI_ANY_SOURCE, CLEAN_RESPONSE, status);

    // Receive the byte stream
    std::vector<int8_t> buf;
    buf.resize(size);
    receive(&buf[0], size * sizeof(char), status.MPI_SOURCE, CLEAN_RESPONSE, status);

    // Decode
    LOFAR::BlobIBufVector<int8_t> bv(buf);
    LOFAR::BlobIStream in(bv);
    int version = in.getStart("cleanresponse");
    ASKAPASSERT(version == 1);
    in >> patchid;
    in >> patch;
    in >> strengthOptimum;
    in.getEnd();
}

int MPIBasicComms::responsible(void)
{
    const int accumulatorStep = 16; // FIXME: Hardcoded elsewhere too
    const int numNodes = getNumNodes();
    const int id = getId();
    int responsible = 0;

    if (id == 0) {
        // Master
        responsible += accumulatorStep - 1; // First n workers
        float accumulators = ceil((float)numNodes / (float)accumulatorStep) - 1.0;
        ASKAPLOG_INFO_STR(logger, "There are " << static_cast<int>(accumulators) 
                << " accumulators.");

        responsible += static_cast<int>(accumulators); // Accumulators 
    } else if (id % accumulatorStep == 0) {
        // Accumulator + worker
        if ((id + accumulatorStep) > numNodes) {
            responsible = numNodes - id - 1;
        } else {
            responsible = accumulatorStep - 1;
        }

    } else {
        // If execution got here, the process is just a worker and is only
        // responsible for itself
        responsible = 0;
    }

    ASKAPLOG_INFO_STR(logger, "I am responsible for " << responsible
            << " processes during accumulation");
    return responsible;
}
