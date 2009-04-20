/// @file
/// @brief cache of UVWMachines
/// @details
/// This class maintains the cache of UVW Machines (tangent point direction is the key). The number
/// of machines cached and the direction tolerance are specified as parameters.
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
///

#include <gridding/UVWMachineCache.h>
#include <askap/AskapError.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief construct the cache
/// @details Set up basic parameters of the cache.
/// @param[in] cacheSize a number of uvw machines in the cache (default is 1)
/// @param[in] tolerance pointing direction tolerance in radians, exceeding which leads 
/// to initialisation of a new UVW Machine
UVWMachineCache::UVWMachineCache(size_t cacheSize, double tolerance) : itsCache(cacheSize),
      itsTangentPoints(cacheSize), itsOldestElement(0), itsTolerance(tolerance)
{
  ASKAPASSERT(cacheSize>=1);
  ASKAPDEBUGASSERT(tolerance>0);
}

/// @brief obtain machine for a particular tangent point
/// @details This is the main method of the class.
/// @param[in] tangent direction to tangent point
/// @return a const reference to uvw machine 
const casa::UVWMachine& UVWMachineCache::machine(const casa::MDirection &tangent) const
{
   return *(itsCache[0]); // temporary stub
}
