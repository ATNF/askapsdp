/// @file PreDifferTaskComms.cc
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
#include "PreDifferTaskComms.h"

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

ASKAP_LOGGER(logger, ".PreDifferTaskComms");

PreDifferTaskComms::PreDifferTaskComms(MPIBasicComms &comms)
    : itsComms(comms)
{
}

PreDifferTaskComms::~PreDifferTaskComms()
{
}

int PreDifferTaskComms::getId(void)
{
    return itsComms.getId();
}

int PreDifferTaskComms::getNumNodes(void)
{
    return itsComms.getNumNodes();
}

void PreDifferTaskComms::abort(void)
{
    itsComms.abort();
}

void PreDifferTaskComms::sendString(const std::string& str, int dest)
{
    return itsComms.sendString(str, dest);
}

std::string PreDifferTaskComms::receiveString(int source)
{
    return itsComms.receiveString(source);
}

std::string PreDifferTaskComms::receiveStringAny(int& source)
{
    return itsComms.receiveStringAny(source);
}

void PreDifferTaskComms::broadcastModel(askap::scimath::Params::ShPtr model)
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
    itsComms.broadcast(&size, sizeof(unsigned long), c_root);

    // Now broadcast the model itself
    itsComms.broadcast(&data[0], data.size() * sizeof(char), c_root);

    ASKAPLOG_INFO_STR(logger, "Broadcast model to all ranks via MPI in "
            << timer.real() << " seconds ");
}

askap::scimath::Params::ShPtr PreDifferTaskComms::receiveModel(void)
{
    // Participate in the broadcast to receive the size of the model
    unsigned long size;
    itsComms.broadcast(&size, sizeof(unsigned long), c_root);

    // Participate in the broadcast to receive the model
    std::vector<char> data;
    data.resize(size);
    itsComms.broadcast(&data[0], data.size() * sizeof(char), c_root);

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

void PreDifferTaskComms::sendNE(askap::scimath::INormalEquations::ShPtr ne_p, int id, int count)
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
    itsComms.send(&size, sizeof(long), id, MPIBasicComms::NORMAL_EQUATION);

    // Now send the actual byte stream
    itsComms.send(&data[0], size * sizeof(char), id, MPIBasicComms::NORMAL_EQUATION);

    ASKAPLOG_INFO_STR(logger, "Sent NormalEquations to rank " << id
            << " via MPI in " << timer.real() << " seconds ");
}

askap::scimath::INormalEquations::ShPtr PreDifferTaskComms::receiveNE(int& id, int& count)
{
    // First receive the size of the byte stream
    long size;
    MPI_Status status;
    itsComms.receive(&size, sizeof(long), MPI_ANY_SOURCE, MPIBasicComms::NORMAL_EQUATION, status);

    // Receive the byte stream
    std::vector<char> data;
    data.resize(size);
    itsComms.receive(&data[0], size * sizeof(char), status.MPI_SOURCE, MPIBasicComms::NORMAL_EQUATION, status);

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

