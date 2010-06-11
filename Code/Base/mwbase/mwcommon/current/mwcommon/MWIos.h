//#  MWIos.h: 
//#
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
//#
//#  $Id$

#ifndef ASKAP_MWCOMMON_MWIOSTREAM_H
#define ASKAP_MWCOMMON_MWIOSTREAM_H

#include <iostream>
#include <fstream>
#include <string>

#define MWCOUT MWIos::os()

namespace askap { namespace mwbase {

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
