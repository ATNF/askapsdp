//#  MWCorrectSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/MWCorrectSpec.h>

using namespace std;

namespace askap { namespace cp {

  void MWCorrectSpec::visit (MWSpecVisitor& visitor) const
  {
    visitor.visitCorrect (*this);
  }

  void MWCorrectSpec::print(ostream& os, const string& indent) const
  {
    printSpec (os, indent, "Correct");
  }

}} // end namespaces
