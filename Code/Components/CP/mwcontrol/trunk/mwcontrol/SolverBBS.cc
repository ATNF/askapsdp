//#  SolverBBS.cc: Solver BBSler of distributed VDS processing
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

#include <mwcontrol/SolverBBS.h>
#include <BBSKernel/Solver.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobString.h>

using namespace LOFAR::BBS;


namespace askap { namespace cp {

  SolverBBS::SolverBBS()
    : itsSolver (0)
  {}

  SolverBBS::~SolverBBS()
  {
    delete itsSolver;
  }

  WorkerProxy::ShPtr SolverBBS::create()
  {
    return WorkerProxy::ShPtr (new SolverBBS());
  }

  void SolverBBS::setInitInfo (const std::string&,
			       const std::string&,
			       const std::string&,
			       const std::string&,
			       unsigned int,
			       bool)
  {
    delete itsSolver;
    itsSolver = 0;
    itsSolver = new Solver();
  }

  int SolverBBS::doProcess (int operation, int streamId,
                            LOFAR::BlobIStream& in,
                            LOFAR::BlobOStream& out)
  {
    return operation;
  }

}} // end namespaces
