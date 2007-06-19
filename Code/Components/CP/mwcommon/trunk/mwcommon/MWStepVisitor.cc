//# MWStepVisitor.cc: Base visitor class to visit an MWStep hierarchy
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWStepVisitor.h>
#include <mwcommon/MWMultiStep.h>
#include <mwcommon/MWError.h>

namespace conrad { namespace cp {

  MWStepVisitor::~MWStepVisitor()
  {}

  void MWStepVisitor::visitMulti (const MWMultiStep& mws)
  {
    for (MWMultiStep::const_iterator it = mws.begin();
	 it != mws.end();
	 ++it) {
      (*it)->visit (*this);
    }
  }

  void MWStepVisitor::visitSolve (const MWSolveStep&)
  {
    CONRADTHROW (MWError,
	       "visitSolve not implemented in derived MWStepVisitor class");
  }

  void MWStepVisitor::visitSubtract (const MWSubtractStep&)
  {
    CONRADTHROW (MWError,
	       "visitSubtract not implemented in derived MWStepVisitor class");
  }

  void MWStepVisitor::visitCorrect (const MWCorrectStep&)
  {
    CONRADTHROW (MWError,
	       "visitCorrect not implemented in derived MWStepVisitor class");
  }

  void MWStepVisitor::visitPredict (const MWPredictStep&)
  {
    CONRADTHROW (MWError,
	       "visitPredict not implemented in derived MWStepVisitor class");
  }

}} // end namespaces
