/// @file
/// @brief Classes to convert a message to/from a blob.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCOMMON_MWBLOBIO_H
#define ASKAP_MWCOMMON_MWBLOBIO_H

#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <casa/OS/Timer.h>
#include <Common/Timer.h>

namespace askap { namespace mwbase {

  /// @ingroup mwcommon
  /// @brief Class to convert a message from a blob.

  /// This class is the opposite of MWBlobOut.
  /// It can be used to obtain operation and streamId and to read the message.

  class MWBlobIn
  {
  public:
    /// Start reading back a message from the buffer. It reads the operation
    /// and streamId which can be obtained using their \a get functions.
    /// The message itself can be read from the \a blobStream().
    explicit MWBlobIn (const LOFAR::BlobString& buf);

    /// Get the operation, streamId, or workerId.
    /// @{
    int getOperation() const
      { return itsOper; }
    int getStreamId() const
      { return itsStreamId; }
    int getWorkerId() const
      { return itsWorkerId; }
    /// @}

    /// Get the timings which can be the low-precision elapsed, system,
    /// and user time, and the high-precision elapsed time.
    /// All times are in seconds.
    /// @{
    float  getElapsedTime() const;
    float  getSystemTime() const;
    float  getUserTime() const;
    double getPrecTime() const;
    /// @}

    /// Get the blobstream to read the data from.
    LOFAR::BlobIStream& blobStream()
      { return itsStream; }

    /// End the Blob processing.
    void finish()
      { itsStream.getEnd(); }

  private:
    LOFAR::BlobIBufString itsBuf;
    LOFAR::BlobIStream    itsStream;
    LOFAR::int32          itsOper;
    LOFAR::int32          itsStreamId;
    LOFAR::int32          itsWorkerId;
    float                 itsElapsedTime;
    float                 itsSystemTime;
    float                 itsUserTime;
    double                itsPrecTime;
  };


  /// @ingroup mwcommon
  /// @brief Class to convert a message to a blob.

  /// This class forms the envelope of messages used in the MW framework.
  /// MW messages are transmitted as blobs.
  /// The envelope consist of the basic blob header with type 'mw'. The
  /// blob header defines things like endianness, version, and length.
  /// The envelope also contains the operation type and streamId.
  /// The operation type tells the worker what is has to do.
  /// The streamId is for future use to make it possible to have parallel
  /// work streams in a worker to keep it busy.
  /// The workerId gives the id of the worker.
  ///
  /// The envelope has room for timings. In this way the master can know
  /// how much time it took for a worker to execute a command.
  /// The \a setTimes function can be used to set the timings. It uses
  /// the casa Timer class to get the low-precision elapsed, user, and system
  /// times. The LOFAR NSTimer class is used for high-precision elapsed time.
  ///
  /// The message proper has to be written by the user of this class
  /// into the supplied \a blobStream.
  /// After all data are written, the \a finish function has to be called.

  class MWBlobOut
  {
  public:
    /// Start a message blob in the buffer and put the given operation,
    /// streamId, and workerId into it.
    /// The message itself can be put into the \a blobStream().
    MWBlobOut (LOFAR::BlobString& buf,
               int operation, int streamId, int workerId=-1);

    /// Reset the operation.
    void setOperation (int operation);

    /// Set the times it took to do the operation.
    void setTimes (const casa::Timer&, const LOFAR::NSTimer&);

    /// Get the blobstream to write the data in.
    LOFAR::BlobOStream& blobStream()
      { return itsStream; }

    /// End the Blob processing.
    void finish()
      { itsStream.putEnd(); }

  private:
    LOFAR::BlobOBufString itsBuf;
    LOFAR::BlobOStream    itsStream;
    //# Remember where to put operation or times.
    int                   itsOperOffset;
    int                   itsTimeOffset;
  };

}} //end namespaces

#endif
