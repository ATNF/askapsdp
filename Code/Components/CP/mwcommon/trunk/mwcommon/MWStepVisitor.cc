//# MWStepVisitor.cc: Base visitor class to visit an MWStep hierarchy
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWStepVisitor.h>
#include <mwcommon/MWMultiStep.h>
#include <mwcommon/MWSolveStep.h>
#include <mwcommon/MWSimpleStep.h>
#include <mwcommon/MWError.h>

namespace conrad { namespace cp {

  MWStepVisitor::~MWStepVisitor()
  {}

  void MWStepVisitor::registerVisit (const std::string& name, VisitFunc* func)
  {
    itsMap[name] = func;
  }

  void MWStepVisitor::visitMulti (const MWMultiStep& mws)
  {
    for (MWMultiStep::const_iterator it = mws.begin();
	 it != mws.end();
	 ++it) {
      (*it)->visit (*this);
    }
  }

  void MWStepVisitor::visitSolve (const MWSolveStep& step)
  {
    visit (step);
  }

  void MWStepVisitor::visitSubtract (const MWSubtractStep& step)
  {
    visitSimple (step);
  }

  void MWStepVisitor::visitCorrect (const MWCorrectStep& step)
  {
    visitSimple (step);
  }

  void MWStepVisitor::visitPredict (const MWPredictStep& step)
  {
    visitSimple (step);
  }

  void MWStepVisitor::visitSimple (const MWSimpleStep& step)
  {
    visit (step);
  }

  void MWStepVisitor::visit (const MWStep& step)
  {
    std::string name = step.className();
    std::map<std::string,VisitFunc*>::const_iterator iter = itsMap.find(name);
    if (iter == itsMap.end()) {
      visitStep (step);
    } else {
      (*iter->second)(*this, step);
    }
  }

  void MWStepVisitor::visitStep (const MWStep& step)
  {
    CONRADTHROW (MWError,
                 "No visit function available for MWStep of type "
                 << step.className());
  }

}} // end namespaces
