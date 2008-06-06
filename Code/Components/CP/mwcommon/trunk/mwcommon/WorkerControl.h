/// @file
/// @brief High level worker control.
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

#ifndef ASKAP_MWCOMMON_WORKERCONTROL_H
#define ASKAP_MWCOMMON_WORKERCONTROL_H

#include <mwcommon/WorkerProxy.h>
#include <mwcommon/MWConnection.h>


namespace askap { namespace cp {

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
