/// @file
/// 
/// @brief helper class to monitor updates of parameters
/// @details It is often needed to monitor a change of some parameters. This class 
/// has comparison operators defined (equal and not equal). Two instances are not
/// equal if they correspond to a different version of tracked parameters. This class
/// can be used in various caching-related implementations where some hierarchy exists
/// (and so different parts of the code can be concerned about changes made at a different time) 
/// It essentially wraps over an integer number which is incremented every time a
/// tracked parameter changes.   
///
///
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef CHANGE_MONITOR_H
#define CHANGE_MONITOR_H

#include <askap/AskapError.h>

namespace askap {

namespace scimath {

/// @brief helper class to monitor updates of parameters
/// @details It is often needed to monitor a change of some parameters. This class 
/// has comparison operators defined (equal and not equal). Two instances are not
/// equal if they correspond to a different version of tracked parameters. This class
/// can be used in various caching-related implementations where some hierarchy exists
/// (and so different parts of the code can be concerned about changes made at a different time) 
/// It essentially wraps over an integer number which is incremented every time a
/// tracked parameter changes.   
/// @ingroup utils
struct ChangeMonitor {
   /// @brief constructor
   inline ChangeMonitor() : itsTag(0) {}
   
   /// @brief check whether two monitors are equal
   /// @details
   /// @param[in] in another change monitor object
   /// @return true if this class and in are equal
   inline bool operator==(const ChangeMonitor &in) const { return in.itsTag == itsTag; }
   
   /// @brief check whether two monitors are not equal
   /// @details
   /// @param[in] in another change monitor object
   /// @return true if this class and in are not equal
   inline bool operator!=(const ChangeMonitor &in) const { return in.itsTag != itsTag; }
    
   /// @brief notify that a change has been made
   /// @details This method is supposed to be called every time a corresponding 
   /// parameter has been updated.
   inline void notifyOfChanges() { ++itsTag; ASKAPCHECK(itsTag!=0, "Overflow detected!");}
      
private: 
   /// @brief integer tag
   /// @details it is incremented every time there is a change in a tracked parameter 
   unsigned long itsTag;
};


} // namespace scimath

} // namespace askap

#endif // #ifndef CHANGE_MONITOR_H

