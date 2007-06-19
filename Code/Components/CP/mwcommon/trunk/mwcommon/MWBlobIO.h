/// @file
/// @brief Classes to convert a message to/from a blob.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MWBLOBIO_H
#define CONRAD_MWCOMMON_MWBLOBIO_H

#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

namespace conrad { namespace cp {

  /// Class to convert a message to a blob.

  /// This class forms the envelope of messages used in the MW framework.
  /// MW messages are transmitted as blobs.
  /// The envelope consist of the basic blob header with type 'mw'. The
  /// blob header defines things like endianness, version, and length.
  /// The envelope also contains the operation type and streamId.
  /// The operation type tells the worker what is has to do.
  /// The streamId is for future use to make it possible to have parallel
  /// work streams in a worker to keep it busy.
  ///
  /// The message proper has to be written by the user of this class
  /// into the supplied \a blobStream.
  /// After all data are written, the \a finish function has to be called.

  class MWBlobOut
  {
  public:
    /// Start a message blob in the buffer and put the given operation
    /// and streamId into it.
    /// The message itself can be put into the \a blobStream().
    MWBlobOut (LOFAR::BlobString& buf, int operation, int streamId);

    /// Get the blobstream to write the data in.
    LOFAR::BlobOStream& blobStream()
      { return itsStream; }

    /// End the Blob processing.
    void finish()
      { itsStream.putEnd(); }

  private:
    LOFAR::BlobOBufString itsBuf;
    LOFAR::BlobOStream    itsStream;
  };


  /// Class to convert a message from a blob.

  /// This class is the opposite of MWBlobOut.
  /// It can be used to obtain operation and streamId and to read the message.

  class MWBlobIn
  {
  public:
    /// Start reading back a message from the buffer. It reads the operation
    /// and streamId which can be obtained using their \a get functions.
    /// The message itself can be read from the \a blobStream().
    explicit MWBlobIn (const LOFAR::BlobString& buf);

    /// Get the operation and streamId.
    /// @{
    int getOperation() const
      { return itsOper; }
    int getStreamId() const
      { return itsStreamId; }
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
  };

}} //end namespaces

#endif
