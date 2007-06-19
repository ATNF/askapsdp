//#  MWSubtractSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/MWSubtractSpec.h>

using namespace std;

namespace conrad { namespace cp {

  void MWSubtractSpec::visit (MWSpecVisitor& visitor) const
  {
    visitor.visitSubtract (*this);
  }

  void MWSubtractSpec::print(ostream& os, const string& indent) const
  {
    printSpec (os, indent, "Subtract");
  }

}} // end namespaces
