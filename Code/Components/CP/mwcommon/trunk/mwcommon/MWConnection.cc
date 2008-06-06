//# MWConnection.cc: Abstract base class for all MWConnections
//#
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
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWConnection.h>
#include <Blob/BlobString.h>
#include <Blob/BlobHeader.h>


namespace askap { namespace cp {

  MWConnection::~MWConnection()
  {}

  void MWConnection::init()
  {}

  bool MWConnection::isConnected() const
  {
    return true;
  }

  void MWConnection::read (LOFAR::BlobString& buf)
  {
    // Try to get the length of the message.
    // If it succeeds, read the data.
    int msgLen = getMessageLength();
    if (msgLen > 0) {
      buf.resize (msgLen);
      receive (buf.data(), msgLen);
    } else {
      // Otherwise read blob header first to get the length.
      LOFAR::BlobHeader hdr;
      receive (&hdr, sizeof(hdr));
      msgLen = hdr.getLength();
      buf.resize (msgLen);
      memcpy (buf.data(), &hdr, sizeof(hdr));
      receive (buf.data() + sizeof(hdr), msgLen-sizeof(hdr));
    }
  }

  void MWConnection::write (const LOFAR::BlobString& buf)
  {
    send (buf.data(), buf.size());
  }

}} // end namespaces
