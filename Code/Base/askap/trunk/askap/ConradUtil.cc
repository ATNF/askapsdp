//#  ConradUtil.h: Common CONRAD utility functions and classes
//#
//#  Copyright (C) 2007
//#
//#  $Id$


#include <algorithm>
#include <unistd.h>
#include <askap/ConradUtil.h>

namespace conrad {

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
