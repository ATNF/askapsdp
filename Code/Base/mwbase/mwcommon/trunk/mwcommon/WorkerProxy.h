/// @file
/// @brief Abstract base class for all worker proxies.
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

#ifndef ASKAP_MWCOMMON_WORKERPROXY_H
#define ASKAP_MWCOMMON_WORKERPROXY_H

#include <mwcommon/WorkerInfo.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

//# Forward Declarations.
namespace LOFAR {
  class BlobString;
  class BlobIStream;
}


namespace askap { namespace mwbase {

  /// @ingroup mwcommon
  /// @brief Abstract base class for all worker proxies.

  /// This class is the abstract base class for the possible workers.
  /// Usually a worker is a proxy class to a class doing the actual work.
  /// The WorkerControl class uses a WorkerProxy to do the actual work.
  ///
  /// Functions to create a worker proxy from a given type name can be
  /// registered in a WorkerFactory object. It gives the user the freedom
  /// to choose which function is registered making it possible to use
  /// some simple test classes instead of the full-blown real classes to
  /// test the control flow.

  class WorkerProxy
  {
  public:
    /// Define a shared pointer to this object.
    typedef boost::shared_ptr<WorkerProxy> ShPtr;

    WorkerProxy();

    virtual ~WorkerProxy();

    /// Fill the buffer with the worker proxy info (like host and work types).
    /// This is used at initialisation time to make the worker capabilities
    /// known to the master.
    void putWorkerInfo (LOFAR::BlobString& out);

    /// Get the worker info from the blob string. It is used by the master
    /// to extract it from a message.
    static WorkerInfo getWorkerInfo (LOFAR::BlobString& in);

    /// Process the command and data that has been received in the input
    /// buffer and write the possible result into the output buffer.
    /// If the input buffer contains the \a quit command, the \a quit function
    /// is called and the status \a false is returned.
    /// Otherwise the \a process function is called to do the actual
    /// processing.
    bool handleMessage (const LOFAR::BlobString& in, LOFAR::BlobString& out);

  private:
    /// Get the work types supported by the proxy.
    virtual std::vector<int> getWorkTypes() const = 0;

    /// Let a derived class process the received data.
    /// The returned operation will be put into the reply message.
    /// If the returned operation is < 0, no reply message will be sent.
    virtual int process (int operation, int streamId,
                         LOFAR::BlobIStream& in,
                         LOFAR::BlobOStream& out) = 0;

    /// Let a derived class end its processing.
    /// The default implementation does nothing.
    virtual void quit();


    /// The workerId is set at the beginning.
    int itsWorkerId;
  };

}} /// end namespaces

#endif
