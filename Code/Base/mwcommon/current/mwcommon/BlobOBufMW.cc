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

// ASKAPSoft includes
#include "askap/AskapError.h"
#include "Common/LofarTypes.h"
#include "Blob/BlobOBuffer.h"

// Local package includes
#include "mwcommon/AskapParallel.h"

using namespace askap::mwcommon;


BlobOBufMW::BlobOBufMW(AskapParallel& comms) :
    itsComms(comms)
{
}

BlobOBufMW::~BlobOBufMW()
{
}

// Put the requested nr of bytes.
LOFAR::uint64 BlobOBufMW::put(const void* buffer, LOFAR::uint64 nbytes)
{
    ASKAPTHROW(AskapError, "Not implemented");
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

