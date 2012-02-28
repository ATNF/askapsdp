/// @file BlobOBufMW.cc
///
/// @copyright (c) 2011 CSIRO
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
#include "BlobOBufMW.h"

// System includes
#include <algorithm> 
#include <iterator> 

// ASKAPSoft includes
#include "askap/AskapError.h"
#include "Common/LofarTypes.h"
#include "Blob/BlobHeader.h"

// Local package includes
#include "askapparallel/AskapParallel.h"

using namespace askap::askapparallel;

BlobOBufMW::BlobOBufMW(AskapParallel& comms, int seqnr, size_t maxBufSize)
    : itsComms(comms), itsSeqNr(seqnr), itsMaxBufSize(maxBufSize)
{
    ASKAPCHECK(itsComms.isParallel(),
            "This class cannot be used in non parallel applications");
}

BlobOBufMW::~BlobOBufMW()
{
    flushBuffer();
}

// Put the requested nbytes of bytes.
LOFAR::uint64 BlobOBufMW::put(const void* buffer, LOFAR::uint64 nbytes)
{
    // 1: Check for zero size request
    if (nbytes == 0) {
        return nbytes;
    }

    // 2: If the current itsBuffer plus the current "put" exceeds itsMaxBufSize
    // flush the buffer.
    if (itsBuffer.size() + nbytes > itsMaxBufSize) {
        flushBuffer();
    }

    // 3: If the current "put" is larger than itsMaxBufSize then just send
    // it directly out of the buffer supplied, otherwise copy it to
    // itsBuffer for sending later
    if (nbytes > itsMaxBufSize) {
        send(buffer, nbytes);
    } else {
        itsBuffer.insert(itsBuffer.end(),
                reinterpret_cast<const char*>(buffer),
                reinterpret_cast<const char*>(buffer) + nbytes);
    }

    // 4: Finally, if the buffer concludes with the end-of-blob value
    // flush the buffer
    if (isEndOfBlob(buffer, nbytes)) {
        flushBuffer();
    }

    return nbytes;
}

// Get the position in the stream.
// -1 is returned if the stream is not seekable.
LOFAR::int64 BlobOBufMW::tellPos() const
{
    return -1;
}

// Set the position in the stream.
// It returns the new position which is -1 if the stream is not seekable.
LOFAR::int64 BlobOBufMW::setPos(LOFAR::int64 pos)
{
    return -1;
}

void BlobOBufMW::send(const void* buffer, size_t nbytes)
{
    // This const_cast relies on the fact that the MPI_send() is allowed
    // to reshuffle the data for sending but should put it back the way
    // it origninally was before returning. Thus this function call has
    // the appearance of treating "buffer" as a const.

    itsComms.send(&nbytes, sizeof(LOFAR::uint64), itsSeqNr);
    if (nbytes > 0) {
        itsComms.send(buffer, nbytes, itsSeqNr);
    }
}

void BlobOBufMW::flushBuffer(void)
{
    if (!itsBuffer.empty()) {
        send(&itsBuffer[0], itsBuffer.size());
        itsBuffer.clear();
    }
    ASKAPCHECK(itsBuffer.empty(), "Buffer flushed but not empty");
}

bool BlobOBufMW::isEndOfBlob(const void* buffer, size_t nbytes)
{
    const LOFAR::uint32* lastInt = reinterpret_cast<const LOFAR::uint32*>(
            reinterpret_cast<const char*>(buffer) + nbytes - sizeof(LOFAR::uint32));
    return (*lastInt == LOFAR::BlobHeader::eobMagicValue());
}
