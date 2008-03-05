/// @file
/// @brief Class to hold a set of MPI connections.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
/// $Id$

#ifndef ASKAP_MWCOMMON_MPICONNECTIONSET_H
#define ASKAP_MWCOMMON_MPICONNECTIONSET_H

#include <mwcommon/MWConnectionSet.h>
#include <mwcommon/MPIConnection.h>
#include <vector>


namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Class to hold a set of MPI connections.

  /// This class represents a set of MPI connections. Typically it is used
  /// to group connections to workers of a specific type.
  /// The main reason for having this class is the ability to check if any
  /// connection in the group is ready to receive data (i.e. if the other
  /// side of the connection has sent data). This is done using MPI_Probe
  /// with the tag of the first connection, so all connections in the group
  /// should have the same (and unique) tag.
  ///
  /// @todo Implement getReadyConnection.

  class MPIConnectionSet: public MWConnectionSet
  {
  public:
    /// Define a shared pointer to this object.
    typedef boost::shared_ptr<MPIConnectionSet> ShPtr;

    /// Set up a connection set to destinations using MPI.
    MPIConnectionSet();

    virtual ~MPIConnectionSet();

    /// Clone the derived object to contain only the connections
    /// as indexed in the given vector.
    virtual MWConnectionSet::ShPtr clone(const std::vector<int>&) const;

    /// Add a connection to the given rank using the tag.
    /// The tag can be used to define the type of destination
    /// (e.g. prediffer or solver).
    /// It returns the sequence nr of the connection.
    int addConnection (int rank, int tag);

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
    std::vector<MPIConnection::ShPtr> itsConns;
  };

}} /// end namespaces

#endif
