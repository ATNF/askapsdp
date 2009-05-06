/// @file SolverTaskComms.cc
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
#include "SolverTaskComms.h"

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

// Local includes
#include <distributedimager/MPIBasicComms.h>

using namespace askap::cp;
using namespace askap::scimath;

ASKAP_LOGGER(logger, ".SolverTaskComms");

SolverTaskComms::SolverTaskComms(MPIBasicComms &comms)
    : itsComms(comms)
{
}

SolverTaskComms::~SolverTaskComms()
{
}

int SolverTaskComms::getId(void)
{
    return itsComms.getId();
}

int SolverTaskComms::getNumNodes(void)
{
    return itsComms.getNumNodes();
}

void SolverTaskComms::abort(void)
{
    itsComms.abort();
}

void SolverTaskComms::sendString(const std::string& str, int dest)
{
    return itsComms.sendString(str, dest);
}

std::string SolverTaskComms::receiveString(int source)
{
    return itsComms.receiveString(source);
}

std::string SolverTaskComms::receiveStringAny(int& source)
{
    return itsComms.receiveStringAny(source);
}

void SolverTaskComms::sendCleanRequest(int patchid,
        const casa::Array<float>& dirty,
        const casa::Array<float>& psf,
        const casa::Array<float>& mask,
        const casa::Array<float>& model,
        double threshold,
        std::string thresholdUnits,
        double fractionalThreshold,
        //std::vector<float>& scales,
        casa::Vector<float>& scales,
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
   itsComms.send(&size, sizeof(long), dest, MPIBasicComms::CLEAN_REQUEST);

    // Now send the actual byte stream
    itsComms.send(&buf[0], size * sizeof(int8_t), dest, MPIBasicComms::CLEAN_REQUEST);

    ASKAPLOG_INFO_STR(logger, "Sent CleanRequest to rank " << dest
            << " via MPI in " << timer.real() << " seconds ");
}

void SolverTaskComms::recvCleanRequest(int& patchid,
        casa::Array<float>& dirty,
        casa::Array<float>& psf,
        casa::Array<float>& mask,
        casa::Array<float>& model,
        double& threshold,
        std::string& thresholdUnits,
        double& fractionalThreshold,
        //std::vector<float>& scales,
        casa::Vector<float>& scales,
        int& niter,
        double& gain)
{
    // First receive the size of the byte stream
    long size;
    MPI_Status status;
    itsComms.receive(&size, sizeof(long), MPI_ANY_SOURCE, MPIBasicComms::CLEAN_REQUEST, status);

    // Receive the byte stream
    std::vector<int8_t> buf;
    buf.resize(size);
    itsComms.receive(&buf[0], size * sizeof(int8_t), status.MPI_SOURCE, MPIBasicComms::CLEAN_REQUEST, status);

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

void SolverTaskComms::sendCleanResponse(int patchid,
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
    itsComms.send(&size, sizeof(long), dest, MPIBasicComms::CLEAN_RESPONSE);

    // Now send the actual byte stream
    itsComms.send(&buf[0], size * sizeof(int8_t), dest, MPIBasicComms::CLEAN_RESPONSE);

    ASKAPLOG_INFO_STR(logger, "Sent CleanResponse to rank " << dest
            << " via MPI in " << timer.real() << " seconds ");
}

void SolverTaskComms::recvCleanResponse(int& patchid,
        casa::Array<float>& patch,
        double& strengthOptimum)
{
    // First receive the size of the byte stream
    long size;
    MPI_Status status;
    itsComms.receive(&size, sizeof(long), MPI_ANY_SOURCE, MPIBasicComms::CLEAN_RESPONSE, status);

    // Receive the byte stream
    std::vector<int8_t> buf;
    buf.resize(size);
    itsComms.receive(&buf[0], size * sizeof(char), status.MPI_SOURCE, MPIBasicComms::CLEAN_RESPONSE, status);

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
