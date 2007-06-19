//# MWConnection.cc: Abstract base class for all MWConnections
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWConnection.h>
#include <Blob/BlobString.h>
#include <Blob/BlobHeader.h>


namespace conrad { namespace cp {

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
