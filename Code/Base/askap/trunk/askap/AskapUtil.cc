//#  AskapUtil.h: Common ASKAP utility functions and classes
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


#include <algorithm>
#include <unistd.h>
#include <askap/AskapUtil.h>

namespace askap {

  std::string getHostName(bool full) {
    char hname[256];
    if (gethostname(hname, 256) != 0) {
      return std::string("localhost");
    }
    std::string hostname(hname);
    if (!full) {
      unsigned int dotloc = hostname.find_first_of(".");
      if (  dotloc != hostname.npos ) {
        return hostname.substr(0,dotloc);
      }
    }
    return hostname;
  }

  std::string toUpper(std::string str)
  {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
  }
  
  std::string toLower(std::string str)
  {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
  }
  
  int nint(double x)
  {
    return x>0 ? int(x+0.5) : int(x-0.5);
  }
  
  int nint(float x)
  {
    return x>0 ? int(x+0.5) : int(x-0.5);
  }
  
} // end namespaces
