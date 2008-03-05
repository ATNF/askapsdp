/// @file
/// @brief Connection to workers based on memory.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCOMMON_MEMCONNECTION_H
#define ASKAP_MWCOMMON_MEMCONNECTION_H

#include <mwcommon/MWConnection.h>
#include <mwcommon/WorkerProxy.h>
#include <Blob/BlobString.h>
#include <boost/shared_ptr.hpp>


namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Connection to workers based on memory.

  /// This class acts as the MW communication mechanism in memory.
  /// It makes it possible to use the MW framework in a single process
  /// which makes debugging easier.
  ///
  /// It is used in the same way as a SocketConnection or MPIConnection, but
  /// because everything is synchronous in a single process, a WorkerProxy
  /// object must be registered with the connection. Its \a handleData function
  /// function is immediately called when data are sent.
  /// The result is stored in a buffer in the MemConnection object, which
  /// can thereafter be read.
  /// After a read the buffer is cleared to ensure it is not read twice
  /// (as is also the case in a 'normal' connection).

  class MemConnection: public MWConnection
  {
  public:
    /// Define a shared pointer to this object.
    typedef boost::shared_ptr<MemConnection> ShPtr;

    /// Set up a connection to the given destination and attach a worker.
    explicit MemConnection (const WorkerProxy::ShPtr& worker);

    virtual ~MemConnection();

    /// Get the length of the message.
    /// It returns the length of the data in the result buffer.
    virtual int getMessageLength();

    /// Receive the data (i.e. the result of a worker from \a itsResult).
    /// The \a itsResult buffer is cleared hereafter.
    virtual void receive (void* buf, unsigned size);

    /// Write the data and process it by the worker.
    /// The result is stored in \a itsResult.
    virtual void write (const LOFAR::BlobString& buf);

  private:
    /// This function cannot be called as \a write is implemented.
    virtual void send (const void* buf, unsigned size);

    WorkerProxy::ShPtr itsWorker;
    LOFAR::BlobString  itsResult;
  };

}} /// end namespaces

#endif
