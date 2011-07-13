/// @file
/// @brief Abstract base class for all MWConnections.
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

#ifndef ASKAP_MWCOMMON_MWCONNECTION_H
#define ASKAP_MWCOMMON_MWCONNECTION_H

#include <unistd.h>
#include <boost/shared_ptr.hpp>

//# Forward Declarations
namespace LOFAR {
  class BlobString;
}


namespace askap { namespace mwcommon {

  /// @ingroup mwcommon
  /// @brief Abstract base class for all MWConnections.

  /// This class defines the base class for classes to transport data.
  /// Actually, the data transport is done between two MWConnection objects
  /// of the same type.
  ///
  /// The data are packed in LOFAR Blob objects to support heterogeneous
  /// machines (with different endianness). It also makes it possible to
  /// version the data to make future upgrades possible. Finally as blob
  /// contains a length making it easily possible to support varying length
  /// messages.
  ///
  /// To support varying length messages for both socket and MPI connections,
  /// the length can be determined first. If found, the message length is
  /// known. Otherwise the blob header is read to find the message length.
  /// This is needed because in MPI a message has to be read in one receive,
  /// while sockets have no direct means to determine the message length.
  ///
  /// Derived classes (e.g. MPIConnection) implement the concrete transport
  /// classes.

  class MWConnection
  {
  public:
    /// Define a shared pointer to this object.
    typedef boost::shared_ptr<MWConnection> ShPtr;

    MWConnection()
    {}

    virtual ~MWConnection();

    /// Initialize the Transport; this may for instance open a file,
    /// port or dbms connection.
    /// Default does nothing.
    virtual void init();

    /// Check the state of this MWConnection. Default is true.
    virtual bool isConnected() const;

    /// Receive the data blob sent by the connected MWConnection
    /// and wait until data has been received into \a buf.
    /// The buffer is resized as needed.
    /// By default it uses the functions \a getMessageLength and \a receive
    /// to determine the length of the message and to receive the data.
    virtual void read (LOFAR::BlobString& buf);

    /// Send the data to the connected MWConnection
    /// and wait until the data has been sent.
    /// By default is uses function \a send to send the data.
    virtual void write (const LOFAR::BlobString& buf);

  private:
    /// Cannot make a copy of this object (thus also of derived classes).
    /// @{
    MWConnection (const MWConnection&);
    MWConnection& operator=(const MWConnection&);
    /// @}

    /// Try to get the length of the message.
    /// -1 is returned if it could not determine it.
    /// In such a case the length needs to be read from the blob header.
    virtual int getMessageLength() = 0;

    /// Receive the given amount of data in the buffer.
    /// and wait until data has been received into buf.
    virtual void receive (void* buf, size_t size) = 0;

    /// Send the fixed sized data to the connected MWConnection
    /// and wait until the data has been sent.
    virtual void send (const void* buf, size_t size) = 0;
  };

}} /// end namespaces

#endif
