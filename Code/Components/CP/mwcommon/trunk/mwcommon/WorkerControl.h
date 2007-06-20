/// @file
/// @brief High level worker control.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_WORKERCONTROL_H
#define CONRAD_MWCOMMON_WORKERCONTROL_H

#include <mwcommon/WorkerProxy.h>
#include <mwcommon/MWConnection.h>


namespace conrad { namespace cp {

  /// @ingroup mwcommon
  /// @brief High level worker control.

  /// This class if the high level control of a proxy worker.
  /// The \a init function sets up the connection and does the initialisation.
  /// The \a run function receives commands from the master
  /// control, lets the proxy execute them, and sends replies back.
  /// When the quit command is received, the \a run function will end.

  class WorkerControl
  {
  public:
    /// Construct with the given proxy, that will execute the commands.
    WorkerControl (const WorkerProxy::ShPtr& proxy);

    /// Initialise the connection and send an init message to the master.
    void init (const MWConnection::ShPtr& connection);

    /// Receive and execute messages until an end message is received.
    void run();

  private:
    MWConnection::ShPtr itsConnection;
    WorkerProxy::ShPtr  itsProxy;
  };

}} /// end namespaces

#endif
