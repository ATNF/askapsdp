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

// System includes
#include <vector>

// ASKAPSoft includes
#include "Common/LofarTypes.h"
#include "Blob/BlobIBuffer.h"

// Local package includes
#include "askapparallel/AskapParallel.h"

namespace askap {
namespace askapparallel {

/// @brief An implementation of BlobIBuffer which streams data from the sender
/// directly.
///
/// @details This class does have an internal buffer and will use it to
/// buffer data from the sender. The buffer will only ever grow to the size
/// of the largest object sent via the stream.
class BlobIBufMW : public LOFAR::BlobIBuffer
{
public:
    /// Constructor.
    /// @param[in] comms    class which provides communication
    ///                     functionality.
    /// @param[in] seqnr    the sequence number indicating the destination
    ///                     for the data stream. This relates to the
    ///                     sequence number in the MPIConnectionSet.
    BlobIBufMW(AskapParallel& comms, int seqnr);

    /// Destructor.
    virtual ~BlobIBufMW();

    /// Get the requested nr of bytes.
    virtual LOFAR::uint64 get(void* buffer, LOFAR::uint64 nbytes);

    /// Get the position in the stream.
    /// -1 is returned if the stream is not seekable.
    virtual LOFAR::int64 tellPos() const;

    /// Set the position in the stream.
    /// It returns the new position which is -1 if the stream is not seekable.
    virtual LOFAR::int64 setPos(LOFAR::int64 pos);

private:
    // Utility function to read data from the destination indicated by
    // by itsSeqNr, and write it into the passed buffer
    void receive(void* buffer, size_t nbytes);

    // Class which provides the acutal communication functionality
    AskapParallel& itsComms;

    // The sequence number of the connection. This relates to the
    // MPIConnectionSet sequence number.
    const int itsSeqNr;

    // Internal buffer used to buffer data read from the connection
    std::vector<char> itsBuffer;
};

} // end namespace askapparallel
} // end namespace askap

#endif
