//#  PredifferBBS.cc: Prediffer BBSler of distributed VDS processing
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

#include <mwcontrol/PredifferBBS.h>
#include <BBSKernel/Prediffer.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobString.h>

using namespace LOFAR::BBS;


namespace askap { namespace cp {

  PredifferBBS::PredifferBBS()
    : itsPrediffer (0)
  {}

  PredifferBBS::~PredifferBBS()
  {
    delete itsPrediffer;
  }

  WorkerProxy::ShPtr PredifferBBS::create()
  {
    return WorkerProxy::ShPtr (new PredifferBBS());
  }

  void PredifferBBS::setInitInfo (const std::string& measurementSet,
				  const std::string& inputColumn,
				  const std::string& skyParameterDB,
				  const std::string& instrumentParameterDB,
				  unsigned int subBand,
				  bool calcUVW)
  {
    delete itsPrediffer;
    itsPrediffer = 0;
    itsPrediffer = new Prediffer (measurementSet, inputColumn,
				  skyParameterDB, instrumentParameterDB, "",
				  subBand, calcUVW);
  }

  int PredifferBBS::doProcess (int operation, int streamId,
                               LOFAR::BlobIStream& in,
                               LOFAR::BlobOStream& out)
  {
    return operation;
  }

}} // end namespaces
