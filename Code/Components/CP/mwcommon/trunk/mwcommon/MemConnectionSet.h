/// @file
/// @brief Set of Memory connections.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MEMCONNECTIONSET_H
#define CONRAD_MWCOMMON_MEMCONNECTIONSET_H

#include <mwcommon/MWConnectionSet.h>
#include <mwcommon/MemConnection.h>
#include <vector>


namespace conrad { namespace cp {

  /// Set of Memory connections.

  /// This class represents a set of memory connections. Typically it is used
  /// to group connections to workers of a specific type.
  /// In practice memory connections will hardly be used, but they come
  /// in handy for debugging purposes.

  class MemConnectionSet: public MWConnectionSet
  {
  public:
    /// Set up a connection set to destinations with the given tag.
    /// The tag can be used to define the type of destination
    /// (e.g. prediffer or solver).
    explicit MemConnectionSet();

    virtual ~MemConnectionSet();

    /// Clone the derived object to contain only the connections
    /// as indexed in the given vector.
    virtual MWConnectionSet::ShPtr clone(const std::vector<int>&) const;

    /// Add a connection to the given worker.
    /// It returns the sequence nr of the connection.
    int addConnection (const WorkerProxy::ShPtr& worker);

    /// Get the number of connections.
    virtual int size() const;

    /// Get seqnr of connection that is ready to receive.
    /// Is not really useful for this type of connection, so always returns 0.
    virtual int getReadyConnection();

    /// Read the data into the BlobString buffer using the connection
    /// with the given sequence nr.
    virtual void read (int seqnr, LOFAR::BlobString&);

    /// Write the data from the BlobString buffer using the connection
    /// with the given sequence nr.
    virtual void write (int seqnr, const LOFAR::BlobString&);

    /// Write the data from the BlobString buffer to all connections.
    virtual void writeAll (const LOFAR::BlobString&);

  private:
    std::vector<MemConnection::ShPtr> itsConns;
  };

}} /// end namespaces

#endif
