/// @file BlobIBufMW.h
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

#ifndef ASKAP_MWCOMMON_BLOGIBUFMW_H
#define ASKAP_MWCOMMON_BLOGIBUFMW_H

// ASKAPSoft includes
#include "Common/LofarTypes.h"
#include "Blob/BlobIBuffer.h"

// Local package includes
#include "mwcommon/AskapParallel.h"

namespace askap {
namespace mwcommon {

class BlobIBufMW : public LOFAR::BlobIBuffer
{
public:
    // Constructor.
    BlobIBufMW(AskapParallel& comms);

    // Destructor.
    virtual ~BlobIBufMW();

    // Get the requested nr of bytes.
    virtual LOFAR::uint64 get(void* buffer, LOFAR::uint64 nbytes);

    // Get the position in the stream.
    // -1 is returned if the stream is not seekable.
    virtual LOFAR::int64 tellPos() const;

    // Set the position in the stream.
    // It returns the new position which is -1 if the stream is not seekable.
    virtual LOFAR::int64 setPos(LOFAR::int64 pos);

private:
    AskapParallel& itsComms;
};


} // end namespace mwcommon
} // end namespace askap

#endif
