//#  BBSProxy.cc: 
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
//#
//#  $Id$

#include <mwcontrol/BBSProxy.h>
#include <mwcommon/MasterControl.h>
#include <Blob/BlobIStream.h>

namespace askap { namespace cp {

  BBSProxy::~BBSProxy()
  {}

  int BBSProxy::process (int operation, int streamId,
                         LOFAR::BlobIStream& in,
                         LOFAR::BlobOStream& out)
  {
    int resOper = operation;
    if (operation == MasterControl::Init) {
      std::string msName, msSuffix, colName, skyDB, instDB;
      LOFAR::uint32 subBand;
      bool calcUVW;
      in >> msName >> msSuffix >> colName >> skyDB >> instDB
	 >> subBand >> calcUVW;
      setInitInfo (msName+msSuffix, colName, skyDB, instDB,
		   subBand, calcUVW);
    } else {
      resOper = doProcess (operation, streamId, in, out);
    }
    return resOper;
  }

}} // end namespaces
