//#  SolverTest.cc: Solver BBSler of distributed VDS processing
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

#include "SolverTest.h"
#include "MWIos.h"
#include <mwcontrol/MWSolveStepBBS.h>
#include <mwcommon/MasterControl.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

using namespace std;


namespace askap { namespace cp {

  SolverTest::SolverTest()
  {}

  SolverTest::~SolverTest()
  {}

  WorkerProxy::ShPtr SolverTest::create()
  {
    return WorkerProxy::ShPtr (new SolverTest());
  }

  void SolverTest::setInitInfo (const std::string& measurementSet,
				const std::string& inputColumn,
				const std::string& skyParameterDB,
				const std::string& instrumentParameterDB,
				unsigned int subBand,
				bool calcUVW)
  {
    MWCOUT << "SolverTest::setInitInfo" << endl
           << "  MS:         " << measurementSet << endl
           << "  Column:     " << inputColumn  << endl
           << "  SkyParmDB:  " << skyParameterDB << endl
           << "  InstParmDB: " << instrumentParameterDB << endl
           << "  Subband:    " << subBand << endl
           << "  CalcUVW:    " << calcUVW << endl;
  }

  int SolverTest::doProcess (int operation, int streamId,
                             LOFAR::BlobIStream& in,
                             LOFAR::BlobOStream& out)
  {
    int resOper = operation;
    MWCOUT << "SolverTest::doProcess" << endl;
    MWCOUT << "  Operation: " << operation << endl;
    MWCOUT << "  StreamId:  " << streamId << endl;
    switch (operation) {
    case MasterControl::SetWd:
    {
      ObsDomain workDomain;
      in >> workDomain;
      MWCOUT << "  Set work domain: " << workDomain << endl;
      break;
    }
    case MasterControl::Step:
    {
      // A step has to be processed.
      // Only a solve can be processed.
      ASKAPCHECK (in.getNextType() == "MWSolveStepBBS",
		   "SolverTest can only handle an MWSolveStepBBS step");
      MWSolveStepBBS step;
      // Fill it from the blobstream.
      step.fromBlob (in);
      itsMaxIter = step.getMaxIter();
      itsNrIter  = 0;
      MWCOUT << "  Solve maxiter " << itsMaxIter << endl;
      break;
    }
    case MasterControl::ParmInfo:
    {
      // ParmInfo has to be processed.
      bool result;
      in >> result;
      MWCOUT << "  ParmInfo " << result << endl;
      resOper = -1;     // no reply to be sent
      break;
    }
    case MasterControl::GetEq:
    {
      // Equations have to be processed.
      bool result;
      in >> result;
      MWCOUT << "  GetEq " << result << endl;
      resOper = -1;     // no reply to be sent
      break;
    }
    case MasterControl::Solve:
    {
      MWCOUT << "  Solve iteration: " << itsNrIter << endl;
      ++itsNrIter;
      bool converged = itsNrIter>=itsMaxIter;
      out << converged;
      break;
    }
    default:
      ASKAPTHROW (MWError, "SolverTest::doProcess: operation "
		   << operation << " is unknown");
    }
    return resOper;
  }

}} // end namespaces
