/// @file
/// @brief Information about a worker.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_WORKERINFO_H
#define CONRAD_MWCOMMON_WORKERINFO_H

#include <string>
#include <vector>

//# Forward Declarations.
namespace LOFAR {
  class BlobOStream;
  class BlobIStream;
}


namespace conrad { namespace cp {

  /// @ingroup mwcommon
  /// @brief Information about a worker.

  /// This class contains the information describing a worker.
  /// It contains the name of the host it is running on and a vector
  /// with the types of work in can perform. Currently only the first
  /// work type is taken into account.
  ///
  /// @todo Take all work types into account.

  class WorkerInfo
  {
  public:
    /// Creatye empty object.
    WorkerInfo();

    /// Construct the object from the given info.
    WorkerInfo (const std::string& hostName,
                const std::vector<int>& workTypes);

    ~WorkerInfo();

    /// Get the host name.
    const std::string& getHostName() const
      { return itsHostName; }

    /// Get the work types.
    const std::vector<int>& getWorkTypes() const
      { return itsWorkTypes; }

    /// Get the first work type. Returns 0 if no work types.
    int getWorkType() const;

    /// Read or write the info from/into a blob.
    /// @{
    friend LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
                                           const WorkerInfo& info);
    friend LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
                                           WorkerInfo& info);
    /// @}

  private:
    std::string      itsHostName;
    std::vector<int> itsWorkTypes;
  };

}} /// end namespaces

#endif
