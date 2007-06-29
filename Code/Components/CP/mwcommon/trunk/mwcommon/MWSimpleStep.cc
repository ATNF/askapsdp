//# MWSubtractStep.cc: Step to process the MW solve command
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWSimpleStep.h>

namespace conrad { namespace cp {

  MWSimpleStep::~MWSimpleStep()
  {}

  void MWSimpleStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitSimple (*this);
  }


  MWSubtractStep::~MWSubtractStep()
  {}

  void MWSubtractStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitSubtract (*this);
  }


  MWCorrectStep::~MWCorrectStep()
  {}

  void MWCorrectStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitCorrect (*this);
  }


  MWPredictStep::~MWPredictStep()
  {}

  void MWPredictStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitPredict (*this);
  }

}} // end namespaces
