//#  MWIos.cc:
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcommon/MWIos.h>

namespace conrad { namespace cp {

  std::string MWIos::itsName = std::string("pgm.out");
  std::ofstream* MWIos::itsIos = 0;

  void MWIos::makeIos()
  {
    delete itsIos;
    itsIos = 0;
    itsIos = new std::ofstream (itsName.c_str());
  }

}} // end namespaces
