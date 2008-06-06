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

#ifndef ASKAP_MWCONTROL_MWIOSTREAM_H
#define ASKAP_MWCONTROL_MWIOSTREAM_H

#include <iostream>
#include <fstream>
#include <string>

#define MWCOUT MWIos::os()

namespace askap { namespace cp {

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
