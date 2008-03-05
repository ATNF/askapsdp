//# WorkerInfo.cc: 
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/WorkerInfo.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>

using namespace LOFAR;

namespace askap { namespace cp {

  WorkerInfo::WorkerInfo()
  {}

  WorkerInfo::WorkerInfo (const std::string& hostName,
                          const std::vector<int>& workTypes)
    : itsHostName  (hostName),
      itsWorkTypes (workTypes)
  {}

  WorkerInfo::~WorkerInfo()
  {}

  int WorkerInfo::getWorkType() const
  {
    return (itsWorkTypes.size() == 0  ?  0 : itsWorkTypes[0]);
  }

  BlobOStream& operator<< (BlobOStream& bs, const WorkerInfo& info)
  {
    bs.putStart ("info", 1);
    bs << info.itsHostName << info.itsWorkTypes;
    bs.putEnd();
    return bs;
  }

  BlobIStream& operator>> (BlobIStream& bs, WorkerInfo& info)
  {
    int version = bs.getStart ("info");
    ASKAPASSERT (version == 1);
    bs >> info.itsHostName >> info.itsWorkTypes;
    bs.getEnd();
    return bs;
  }

}} // end namespaces
