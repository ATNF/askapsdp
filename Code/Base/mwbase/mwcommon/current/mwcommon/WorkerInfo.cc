/// @file
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


#include <mwcommon/WorkerInfo.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>

using namespace LOFAR;

namespace askap { namespace mwbase {

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
