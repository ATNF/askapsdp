/// @file
/// @brief Abstract base class for all worker proxies.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_WORKERPROXY_H
#define CONRAD_MWCOMMON_WORKERPROXY_H

#include <mwcommon/WorkerInfo.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

//# Forward Declarations.
namespace LOFAR {
  class BlobString;
  class BlobIStream;
}


namespace conrad { namespace cp {

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
    bool handleData (const LOFAR::BlobString& in, LOFAR::BlobString& out);

  private:
    /// Get the work types supported by the proxy.
    virtual std::vector<int> getWorkTypes() const = 0;

    /// Let a derived class process the received data.
    virtual void process (int operation, int streamId,
			  LOFAR::BlobIStream& in,
			  LOFAR::BlobString& out) = 0;

    /// Let a derived class end its processing.
    /// The default implementation does nothing.
    virtual void quit();
  };

}} /// end namespaces

#endif
