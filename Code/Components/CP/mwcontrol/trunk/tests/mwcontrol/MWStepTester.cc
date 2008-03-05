//#  MWStepTester.cc: 
//#
//#  Copyright (C) 2007
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
