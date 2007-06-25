//# MemConnection.cc: Memory connection to a worker
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MemConnection.h>
#include <mwcommon/MWError.h>

namespace conrad { namespace cp {

  MemConnection::MemConnection (const WorkerProxy::ShPtr& worker)
    : itsWorker (worker)
  {}

  MemConnection::~MemConnection()
  {}

  int MemConnection::getMessageLength()
  {
    CONRADCHECK (itsResult.size() > 0,
		 "MemConnection: no result has been received");
    return itsResult.size();
  }

  void MemConnection::receive (void* buf, unsigned size)
  {
    CONRADASSERT (itsResult.size() == size);
    memcpy (buf, itsResult.data(), size);
    // Clear buffer to make sure data cannot be read twice.
    itsResult.resize (0);
  }

  void MemConnection::write (const LOFAR::BlobString& data)
  {
    // Internal buffer must be empty, otherwise no read was done.
    CONRADCHECK (itsResult.size() == 0,
		 "MemConnection: received result has not been read");
    // Let the worker process the data and keep its result.
    itsWorker->handleMessage (data, itsResult);
  }

  void MemConnection::send (const void*, unsigned)
  {
    CONRADTHROW (MWError, "MemConnection::send should not be called");
  }

}} // end namespaces
