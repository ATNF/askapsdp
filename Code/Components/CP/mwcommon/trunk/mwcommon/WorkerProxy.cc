//# WorkerProxy.cc: Base class for the proxy of a worker
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/WorkerProxy.h>
#include <mwcommon/WorkerInfo.h>
#include <mwcommon/MWBlobIO.h>
#include <mwcommon/SocketConnection.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobArray.h>


namespace conrad { namespace cp {

  WorkerProxy::WorkerProxy()
    {}

  WorkerProxy::~WorkerProxy()
    {}

  void WorkerProxy::putWorkerInfo (LOFAR::BlobString& out)
  {
    // Write the work types and the host name of the worker.
    MWBlobOut bout(out, 0, 0);
    WorkerInfo info(SocketConnection::getHostName(), getWorkTypes());
    bout.blobStream() << info;
    bout.finish();
  }

  WorkerInfo WorkerProxy::getWorkerInfo (LOFAR::BlobString& in)
  {
    // Read back from blob string.
    MWBlobIn bin(in);
    CONRADASSERT (bin.getOperation() == 0);
    WorkerInfo info;
    bin.blobStream() >> info;
    return info;
  }

  bool WorkerProxy::handleData (const LOFAR::BlobString& in,
				LOFAR::BlobString& out)
  {
    MWBlobIn bin(in);
    int operation = bin.getOperation();
    if (operation < 0) {
      quit();
    } else {
      process (operation, bin.getStreamId(), bin.blobStream(), out);
    }
    bin.finish();
    return operation >= 0;
  }

  void WorkerProxy::quit()
    {}

}} // end namespaces
