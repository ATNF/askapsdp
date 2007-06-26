//#  MWIos.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_MWIOSTREAM_H
#define CONRAD_MWCONTROL_MWIOSTREAM_H

#include <iostream>
#include <fstream>
#include <string>

#define MWCOUT MWIos::os()

namespace conrad { namespace cp {

  // MPI has the problem that the output of cout is unpredictable.
  // Therefore the output of tMWControl is using a separate output
  // file for each rank.
  // This class makes this possible. The alias MWCOUT can be used for it.
  class MWIos
  {
  public:
    static void setName (const std::string& name)
      { itsName = name; }
    static std::ofstream& os()
      { if (!itsIos) makeIos(); return *itsIos; }

  private:
    static void makeIos();

    static std::string    itsName;
    static std::ofstream* itsIos;
  };

}} // end namespaces

#endif
