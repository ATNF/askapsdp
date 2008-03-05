//# MWStep.cc: Step to process the MW commands
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWStep.h>

namespace askap { namespace cp {

  MWStep::~MWStep()
  {}

  void MWStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visit (*this);
  }

}} // end namespaces
