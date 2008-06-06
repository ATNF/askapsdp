//#  MWStepTester.cc: 
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

#include "MWStepTester.h"
#include "MWIos.h"
#include <mwcommon/MasterControl.h>
#include <mwcontrol/MWSolveStepBBS.h>
#include <mwcommon/AskapUtil.h>

using namespace std;


namespace askap { namespace cp {

  MWStepTester::MWStepTester (int streamId, LOFAR::BlobOStream* out)
    : itsStreamId  (streamId),
      itsOperation (MasterControl::Step),
      itsOut       (out)
  {}

  MWStepTester::~MWStepTester()
  {}

  void MWStepTester::visitSolve (const MWSolveStep& stepmw)
  {
    const MWSolveStepBBS& step = dynamic_cast<const MWSolveStepBBS&>(stepmw);
    MWCOUT << "  MWStepTester::visitSolve,  streamId " << itsStreamId << endl;
    MWCOUT << "   Max nr. of iterations:  " << step.getMaxIter() << endl;
    MWCOUT << "   Convergence threshold:  " << step.getEpsilon() << endl;
    MWCOUT << "   Min fraction converged: " << step.getFraction() << endl;
    MWCOUT << "   Solvable parameters:    " << step.getParmPatterns() << endl;
    MWCOUT << "   Excluded parameters:    " << step.getExclPatterns() << endl;
    MWCOUT << "   Domain shape:           " << step.getDomainShape() << endl;
    itsOperation = MasterControl::ParmInfo;
    *itsOut << true;
  }

  void MWStepTester::visitCorrect (const MWCorrectStep&)
  {
    MWCOUT << "  MWStepTester::visitCorrect,  streamId "
           << itsStreamId << endl;
    writeResult (true);
  }

  void MWStepTester::visitSubtract (const MWSubtractStep&)
  {
    MWCOUT << "  MWStepTester::visitSubtract,  streamId "
           << itsStreamId << endl;
    writeResult (true);
  }

  void MWStepTester::visitPredict (const MWPredictStep&)
  {
    MWCOUT << "  MWStepTester::visitPredict,  streamId "
           << itsStreamId << endl;
    writeResult (true);
  }

  void MWStepTester::writeResult (bool result)
  {
    *itsOut << result;
  }

}} // end namespaces
