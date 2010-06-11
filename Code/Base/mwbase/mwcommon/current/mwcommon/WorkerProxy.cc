/// @file
/// @brief Base class for the proxy of a worker
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
/// @author Ger van Diepen <diepen@astron.nl>


#include <mwcommon/WorkerProxy.h>
#include <mwcommon/WorkerInfo.h>
#include <mwcommon/MasterControl.h>
#include <mwcommon/MWBlobIO.h>
#include <mwcommon/SocketConnection.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobArray.h>


namespace askap { namespace mwbase {

  WorkerProxy::WorkerProxy()
    : itsWorkerId (-1)
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
    ASKAPASSERT (bin.getOperation() == 0);
    WorkerInfo info;
    bin.blobStream() >> info;
    return info;
  }

  bool WorkerProxy::handleMessage (const LOFAR::BlobString& in,
                                   LOFAR::BlobString& out)
  {
    MWBlobIn bin(in);
    int operation = bin.getOperation();
    if (operation < 0) {
      quit();
    } else {
      // Set the (unique) worker id when initializing.
      if (operation == MasterControl::Init) {
        itsWorkerId = bin.getWorkerId();
      }
      // Create the output blob using the operation of the input.
      // The process function can reset the operation.
      // Do timings of the process functions and put them into the blob.
      MWBlobOut bout(out, bin.getOperation(), bin.getStreamId(), itsWorkerId);
      casa::Timer timer;
      LOFAR::NSTimer precTimer;
      precTimer.start();
      int oper = process (bin.getOperation(), bin.getStreamId(),
                          bin.blobStream(), bout.blobStream());
      // Set the timings.
      precTimer.stop();
      if (oper < 0) {
        // Do not send a reply.
        out.resize (0);
      } else {
        bout.setTimes (timer, precTimer);
        // Reset operation if changed.
        if (oper != bin.getOperation()) {
          bout.setOperation (oper);
        }
        bout.finish();
      }
    }
    bin.finish();
    return operation >= 0;
  }

  void WorkerProxy::quit()
    {}

}} // end namespaces
