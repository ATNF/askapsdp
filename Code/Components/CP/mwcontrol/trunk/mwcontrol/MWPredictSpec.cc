//#  MWPredictSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/MWPredictSpec.h>

using namespace std;

namespace conrad { namespace cp {

  void MWPredictSpec::visit (MWSpecVisitor& visitor) const
  {
    visitor.visitPredict (*this);
  }

  void MWPredictSpec::print(ostream& os, const string& indent) const
  {
    printSpec (os, indent, "Predict");
  }
  
}} // end namespaces
