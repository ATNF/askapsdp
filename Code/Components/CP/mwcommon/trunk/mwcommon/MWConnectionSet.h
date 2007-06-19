/// @file MWConnectionSet.h
/// @brief Abstract base class for all MWConnectionSets.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MWCONNECTIONSET_H
#define CONRAD_MWCOMMON_MWCONNECTIONSET_H

#include <Blob/BlobString.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace conrad { namespace cp {

  /// Abstract base class for all MWConnectionSets.

  /// This class defines the abstract base class for all MWConnectionSet
  /// classes.
  /// The object can be cloned, where it is possible to only use the
  /// given connections. In this way connections can be regrouped asd needed.
  /// Note that a cloned object uses the same MWConnection objects as
  /// the original.
  ///
  /// See class MWConnection for a description of connections.

  class MWConnectionSet
  {
  public:
    /// Define a shared pointer to this object.
    typedef boost::shared_ptr<MWConnectionSet> ShPtr;

    MWConnectionSet()
      {}

    virtual ~MWConnectionSet();

    /// Clone the derived object, optionally to contain only the connections
    /// as indexed in the given vector.
    /// It uses the same connections as the original.
    /// @{
    MWConnectionSet::ShPtr clone() const;
    virtual MWConnectionSet::ShPtr clone(const std::vector<int>&) const = 0;
    /// @}

    /// Get the number of connections.
    virtual int size() const = 0;

    /// Get seqnr of connection that is ready to receive.
    /// <0 means no connection ready yet.
    virtual int getReadyConnection() = 0;

    /// Read the data into the BlobString buffer using the connection
    /// with the given sequence nr.
    virtual void read (int seqnr, LOFAR::BlobString&) = 0;

    /// Write the data from the BlobString buffer using the connection
    /// with the given sequence nr.
    virtual void write (int seqnr, const LOFAR::BlobString&) = 0;

    /// Write the data from the BlobString buffer to all connections.
    virtual void writeAll (const LOFAR::BlobString&) = 0;
  };

}} /// end namespaces

#endif
