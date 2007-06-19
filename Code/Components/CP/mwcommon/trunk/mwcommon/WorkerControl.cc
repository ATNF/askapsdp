//# WorkerControl.cc: Worker connection of distributed VDS processing
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/WorkerControl.h>
#include <Blob/BlobString.h>

using namespace std;


namespace conrad { namespace cp {

  WorkerControl::WorkerControl (const WorkerProxy::ShPtr& proxy)
    : itsProxy (proxy)
  {}

  void WorkerControl::init (const MWConnection::ShPtr& connection)
  {
    itsConnection = connection;
  }

  void WorkerControl::run()
  {
    LOFAR::BlobString bufIn, bufOut;
    // Start with sending the work types.
    itsProxy->putWorkerInfo (bufOut);
    itsConnection->write (bufOut);
    // Read data until an end command is received.
    while (true) {
      bufIn.resize (0);
      bufOut.resize (0);
      itsConnection->read (bufIn);
      if (! itsProxy->handleData (bufIn, bufOut)) {
	break;
      }
      if (bufOut.size() > 0) {
	itsConnection->write (bufOut);
      }
    }
  }

}} // end namespaces
