//# WorkDomainSpec.cc: Define the specifications of the work domain
//#
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
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/WorkDomainSpec.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobArray.h>

using namespace std;


namespace askap { namespace mwbase {

  void WorkDomainSpec::setAntennas (const vector<int>& antNrs)
  {
    itsAntNrs = antNrs;
  }

  void WorkDomainSpec::setAntennaNames (const vector<string>& antNames)
  {
    itsAntNames = antNames;
  }

  void WorkDomainSpec::setCorr (const vector<bool>& corr)
  {
    itsCorr = corr;
  }

  LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
				  const WorkDomainSpec& wds)
  {
    bs.putStart ("WDS", 1);
    bs << wds.itsInColumn
       << wds.itsAntNrs
       << wds.itsAntNames
       << wds.itsAutoCorr
       << wds.itsCorr
       << wds.itsShape
       << wds.itsFreqInt
       << wds.itsTimeInt;
    return bs;
  }

  LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
				  WorkDomainSpec& wds)
  {
    int vers = bs.getStart ("WDS");
    ASKAPASSERT (vers == 1);
    bs >> wds.itsInColumn
       >> wds.itsAntNrs
       >> wds.itsAntNames
       >> wds.itsAutoCorr
       >> wds.itsCorr
       >> wds.itsShape
       >> wds.itsFreqInt
       >> wds.itsTimeInt;
    return bs;
  }


}} // end namespaces
