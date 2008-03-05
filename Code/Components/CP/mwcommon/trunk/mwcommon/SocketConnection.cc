//# SocketConnection.cc: Connection to workers based on a socket
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/SocketConnection.h>
#include <mwcommon/MWError.h>
#include <unistd.h>             // for gethostname


namespace askap { namespace cp {

  SocketConnection::SocketConnection (const std::string& hostName,
                                      const std::string& port)
    : itsConnSocket ("mwsck", hostName, port),
      itsDataSocket (0)
  {}

  SocketConnection::SocketConnection (LOFAR::Socket* conn)
    : itsDataSocket (conn)
  {}

  SocketConnection::~SocketConnection()
  {
    if (itsDataSocket != &itsConnSocket) {
      delete itsDataSocket;
    }
  }

  bool SocketConnection::isConnected() const
  {
    return itsDataSocket  &&  itsDataSocket->isConnected();
  }

  int SocketConnection::getMessageLength()
  {
    return -1;
  }

  void SocketConnection::receive (void* buf, unsigned size)
  {
    if (!itsDataSocket) {
      init();
    }
    char* cbuf = static_cast<char*>(buf);
    while (size > 0) {
      int sz = itsDataSocket->read (cbuf, size);
      ASKAPCHECK (sz>=0,
                   "Read on socket failed: " << itsDataSocket->errstr());
      cbuf += sz;
      size -= sz;
    }
  }

  void SocketConnection::send (const void* buf, unsigned size)
  {
    if (!itsDataSocket) {
      init();
    }
    itsDataSocket->writeBlocking (buf, size);
  }

  void SocketConnection::init()
  {
    // Create a client socket.
    // Try to connect: may fail if no listener started yet.
    // So retry during one minute.
    int status;
    for (int i=0; i<60; ++i) {
      status = itsConnSocket.connect();
      if (status == LOFAR::Socket::SK_OK) {
        // Connected, so socket can be used to send/receive data.
        itsDataSocket = &itsConnSocket;
        break;
      }
      sleep (1);
    }
    ASKAPCHECK (status == LOFAR::Socket::SK_OK,
                 "SocketConnection client could not connect to host "
                 << itsConnSocket.host() << ", port "
                 << itsConnSocket.port()
                 << ", LOFAR::Socket status " << status << ' '
                 << itsConnSocket.errstr());
    ASKAPASSERT (isConnected());
  }

  std::string SocketConnection::getHostName()
  {
    char nm[256];
    ::gethostname(nm, sizeof(nm));
    return std::string(nm);
  }

}} // end namespaces
