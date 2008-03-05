//# SocketListener.cc: Class that creates a socket and accepts connections
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/SocketListener.h>
#include <mwcommon/MWError.h>


namespace askap { namespace cp {

  SocketListener::SocketListener (const std::string& port)
    : itsConnSocket (new LOFAR::Socket("mwsck", port))
  {}

  SocketConnection::ShPtr SocketListener::accept()
  {
    LOFAR::Socket* socket = itsConnSocket->accept();
    SocketConnection::ShPtr dataConn(new SocketConnection(socket));
    int status = itsConnSocket->errcode();
    ASKAPCHECK (socket  &&  status == LOFAR::Socket::SK_OK,
                 "SocketConnection server did not accept on host "
                 << itsConnSocket->host() << ", port " << itsConnSocket->port()
                 << ", LOFAR::Socket status " << status << ' '
                 << itsConnSocket->errstr());
    ASKAPASSERT (dataConn->isConnected());
    return dataConn;
  }

}} // end namespaces
