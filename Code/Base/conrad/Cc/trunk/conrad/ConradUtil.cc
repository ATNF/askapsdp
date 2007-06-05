//#  ConradUtil.h: Common CONRAD utility functions and classes
//#
//#  Copyright (C) 2007
//#
//#  $Id$


#include <algorithm>
#include <conrad/ConradUtil.h>

namespace conrad {

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

} // end namespaces
