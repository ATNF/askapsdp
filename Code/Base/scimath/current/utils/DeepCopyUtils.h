/// @file
/// 
/// @brief helper functions to copy std containers of casa arrays
/// @details casa arrays use reference semantics, therefore one need to copy them
/// explicitly. This file has utilities designed to help with copying of std containers
/// with casa arrays.   
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

#ifndef DEEP_COPY_UTILS_H
#define DEEP_COPY_UTILS_H

#include <map>
#include <casa/Arrays/Array.h>


namespace askap {

namespace scimath {

/// @brief a helper method for a deep copy of casa arrays held in
/// stl maps
/// @details Can be moved to utils if found useful somewhere else
/// @param[in] in input array
/// @param[out] out output array (will be resized)
template<typename Key, typename T>
void deepCopyOfSTDMap(const std::map<Key, T> &in,
                         std::map<Key, T> &out)
{
   out = std::map<Key, T>();
   const typename std::map<Key, T>::const_iterator inEnd = in.end();
   for (typename std::map<Key, T>::const_iterator inIt = in.begin();
        inIt != inEnd; ++inIt) {
        out[inIt->first] = inIt->second.copy();
   }
}

} // namespace scimath

} // namespace askap


#endif // #ifndef DEEP_COPY_UTILS_H

