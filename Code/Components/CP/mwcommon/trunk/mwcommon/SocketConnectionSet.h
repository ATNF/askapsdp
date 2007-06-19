/// @file
/// @brief Set of socket connections.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_SOCKETCONNECTIONSET_H
#define CONRAD_MWCOMMON_SOCKETCONNECTIONSET_H

#include <mwcommon/MWConnectionSet.h>
#include <mwcommon/SocketListener.h>
#include <mwcommon/SocketConnection.h>
#include <vector>


namespace conrad { namespace cp {

  /// Set of socket connections.

  /// This class represents a set of socket connections. Typically it is used
  /// to group connections to workers of a specific type.
  /// The main reason for having this class is the ability to check if any
  /// connection in the group is ready to receive data (i.e. if the other
  /// side of the connection has sent data). This is done using the \a select
  /// function on the fd-s of the sockets in the set.
  ///
  /// The SocketConnectionSet class creates a socket listener. Thus it is
  /// the server side of a connection and is typically used by the master
  /// control.
  ///
  /// @todo Implement getReadyConnection.

  class SocketConnectionSet: public MWConnectionSet
  {
  public:
    /// Set up a connection set for a server.
    /// It creates a SocketListener object on the given port.
    explicit SocketConnectionSet (const std::string& port);

    /// Set up a connection from an existing SocketListener.
    /// It makes a (shallow) copy of the listener object.
    explicit SocketConnectionSet (const SocketListener&);

    virtual ~SocketConnectionSet();

    /// Clone the derived object to contain only the connections
    /// as indexed in the given vector.
    virtual MWConnectionSet::ShPtr clone(const std::vector<int>&) const;

    /// Accept connections from the given number of clients to the server.
    void addConnections (int nr);

    /// Get the number of connections.
    virtual int size() const;

    /// Get seqnr of connection that is ready to receive.
    /// <0 means no connection ready yet.
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
    SocketListener                       itsListener;
    std::vector<SocketConnection::ShPtr> itsConns;
  };

}} /// end namespaces

#endif
