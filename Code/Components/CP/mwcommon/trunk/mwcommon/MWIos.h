//#  MWIos.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef ASKAP_MWCOMMON_MWIOSTREAM_H
#define ASKAP_MWCOMMON_MWIOSTREAM_H

#include <iostream>
#include <fstream>
#include <string>

#define MWCOUT MWIos::os()

namespace askap { namespace cp {

  /// MPI has the problem that the output of cout is unpredictable.
  /// Therefore the output of tMWControl is using a separate output
  /// file for each rank.
  /// This class makes this possible. The alias MWCOUT can be used for it.
  ///
  /// Not that everything is static, so no destructor is called.
  /// The clear function can be called at the end of the program to
  /// delete the internal object, otherwise tools like valgrind will
  /// complain about a memory leak.

  class MWIos
  {
  public:
    /// Define the name of the output file.
    static void setName (const std::string& name)
      { itsName = name; }

    /// Get access to its ostream.
    /// It creates the ostream if not done yet.
    static std::ofstream& os()
      { if (!itsIos) makeIos(); return *itsIos; }

    /// Remove the ostream (otherwise there'll be a memory leak).
    static void clear()
      { delete itsIos; }

  private:
    /// Make the ostream if not done yet.
    static void makeIos();

    static std::string    itsName;
    static std::ofstream* itsIos;
  };

}} // end namespaces

#endif
