//#  MWIos.cc:
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include "MWIos.h"

namespace conrad { namespace cp {

  std::string MWIos::itsName = std::string("pgm.out");
  std::ofstream* MWIos::itsIos = 0;

  void MWIos::makeIos()
  {
    itsIos = new std::ofstream (itsName.c_str());
  }

}} // end namespaces
