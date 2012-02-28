/// @file BlobOBufMW.h
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

#ifndef ASKAP_MWCOMMON_BLOBOBUFMW_H
#define ASKAP_MWCOMMON_BLOBOBUFMW_H

// System includes
#include <vector>

// ASKAPSoft includes
#include "Common/LofarTypes.h"
#include "Blob/BlobOBuffer.h"

// Local package includes
#include "askapparallel/AskapParallel.h"

namespace askap {
namespace askapparallel {

/// @brief An implementation of BlobOBuffer which streams data directly to
/// the destination.
///
/// @details While this class will stream larger buffers to the destination,
/// it will group smaller buffers into a temporary buffer, flushing it when
/// it gets full, or when the end-of-blob is reached.
class BlobOBufMW : public LOFAR::BlobOBuffer
{ 
    public:
        /// Constructor
        /// @param[in] comms    class which provides communication
        ///                     functionality.
        /// @param[in] seqnr    the sequence number indicating the destination
        ///                     for the data stream. This relates to the
        ///                     sequence number in the MPIConnectionSet.
        /// @param[in] maxBufSize   the maximum size of the internal buffer used
        ///                         to group data which has been submitted by
        ///                         put() such that it can be sent in batches.
        ///                         Where buffers larger than "maxBufSize" are
        ///                         passed to put(), that data is sent directly
        ///                         right out of the buffer rather than being
        ///                         copied into the internal buffer.
        BlobOBufMW(AskapParallel& comms, int seqnr,
                size_t maxBufSize = 1048576);

        /// Destructor
        virtual ~BlobOBufMW();

        /// Put the requested nr of bytes.
        /// @return the number of bytes put.
        virtual LOFAR::uint64 put(const void* buffer, LOFAR::uint64 nbytes);

        /// Get the position in the stream.
        /// -1 is returned if the stream is not seekable.
        virtual LOFAR::int64 tellPos() const;

        /// Set the position in the stream.
        /// It returns the new position which is -1 if the stream is not seekable.
        virtual LOFAR::int64 setPos(LOFAR::int64 pos);

    private:
        // Utility function to send the buffer to the destination indicated
        // by itsSeqNr
        void send(const void* buffer, size_t nbytes);

        // When called the contents of itsBuffer will be sent the the
        // intended destination and itsBuffer is emptied.
        void flushBuffer(void);

        // Utility function which checks the tail of the buffer for the
        // end-of-blob value
        // @return true if end-of-blob value is found, otherwise false
        bool isEndOfBlob(const void* buffer, size_t nbytes);

        // Class which provides the acutal communication functionality
        AskapParallel& itsComms;

        // The sequence number of the connection. This relates to the
        // MPIConnectionSet sequence number.
        const int itsSeqNr;

        // The maximum size of the buffer
        const size_t itsMaxBufSize;

        // Internal buffer used to store subsets of the stream so
        // data can be sent in batches
        std::vector<char> itsBuffer;
};

} // end namespace askapparallel
} // end namespace askap

#endif
