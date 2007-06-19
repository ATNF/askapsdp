//# MWSpecVisitor.cc: Base visitor class to visit an MWSpec hierarchy
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <mwcontrol/MWSpecVisitor.h>
#include <mwcontrol/MWMultiSpec.h>
#include <mwcommon/MWError.h>

namespace conrad { namespace cp {

  MWSpecVisitor::~MWSpecVisitor()
  {}

  void MWSpecVisitor::visitMulti (const MWMultiSpec& mws)
  {
    for (MWMultiSpec::const_iterator it = mws.begin();
	 it != mws.end();
	 ++it) {
      (*it)->visit (*this);
    }
  }

  void MWSpecVisitor::visitSolve (const MWSolveSpec&)
  {
    CONRADTHROW (MWError,
	       "visitSolve not implemented in derived MWSpecVisitor class");
  }

  void MWSpecVisitor::visitSubtract (const MWSubtractSpec&)
  {
    CONRADTHROW (MWError,
	       "visitSubtract not implemented in derived MWSpecVisitor class");
  }

  void MWSpecVisitor::visitCorrect (const MWCorrectSpec&)
  {
    CONRADTHROW (MWError,
	       "visitCorrect not implemented in derived MWSpecVisitor class");
  }

  void MWSpecVisitor::visitPredict (const MWPredictSpec&)
  {
    CONRADTHROW (MWError,
	       "visitPredict not implemented in derived MWSpecVisitor class");
  }

}} // end namespaces
