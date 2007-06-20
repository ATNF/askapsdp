/// @file
/// @brief Class that creates a socket and accepts connections.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_SOCKETLISTENER_H
#define CONRAD_MWCOMMON_SOCKETLISTENER_H

#include <mwcommon/SocketConnection.h>
#include <Common/Net/Socket.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace conrad { namespace cp {

  /// @ingroup mwcommon
  /// @brief Class that creates a socket and accepts connections.

  /// This class sets up a socket listener. It is used by SocketConnectionSet
  /// to accept connection requests from workers.
  ///
  /// Internally the class uses a shared pointer to a socket object.
  /// It means that a copy of a SocketListener object can be made, but that
  /// copies share the same underlying socket object.

  class SocketListener
  {
  public:
    /// Set up the server side of a listener.
    explicit SocketListener (const std::string& port);

    /// Listen to a connection and accept it.
    /// It blocks until another process wants to connect.
    SocketConnection::ShPtr accept();

  private:
    boost::shared_ptr<LOFAR::Socket> itsConnSocket;
  };

}} /// end namespaces

#endif
