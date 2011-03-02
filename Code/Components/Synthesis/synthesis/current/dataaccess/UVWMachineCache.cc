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

#include <dataaccess/UVWMachineCache.h>
#include <askap/AskapError.h>

// for logging
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".dataaccess");


using namespace askap;
using namespace askap::synthesis;

/// @brief construct the cache
/// @details Set up basic parameters of the cache.
/// @param[in] cacheSize a number of uvw machines in the cache (default is 1)
/// @param[in] tolerance pointing direction tolerance in radians, exceeding which leads 
/// to initialisation of a new UVW Machine
UVWMachineCache::UVWMachineCache(size_t cacheSize, double tolerance) : itsCache(cacheSize),
      itsTangentPoints(cacheSize), itsPhaseCentres(cacheSize), 
      itsOldestElement(0), itsTolerance(tolerance)
{
  ASKAPASSERT(cacheSize>=1);
  ASKAPDEBUGASSERT(tolerance>0);
  ASKAPDEBUGASSERT(itsCache.size() == itsTangentPoints.size());
  ASKAPDEBUGASSERT(itsCache.size() == itsPhaseCentres.size());
}

/// @brief destructor to print some stats
/// @details This method writes in the log cache utilisation statistics
UVWMachineCache::~UVWMachineCache()
{
   if (itsCache.size()) {
       size_t cntUsed = 0;
       for (size_t elem=0; elem < itsCache.size(); ++elem) {
            if (!itsCache[elem]) {
                ++cntUsed;
            }
       }
       ASKAPLOG_INFO_STR(logger, "UVW-cache utilisation: used "<<cntUsed<<" cache(s) out of "<<itsCache.size()<<" available");
    }
}


/// @brief obtain machine for a particular tangent point and phase centre
/// @details This is the main method of the class.
/// @param[in] phaseCentre direction to the input phase centre
/// @param[in] tangent direction to tangent point
/// @return a const reference to uvw machine 
const casa::UVWMachine& UVWMachineCache::machine(const casa::MDirection &phaseCentre,
                                                 const casa::MDirection &tangent) const
{   
   const size_t index = getIndex(phaseCentre,tangent);
   boost::shared_ptr<casa::UVWMachine> &machinePtr = itsCache[index];
   if (!machinePtr) {
       // need to set up a new machine here
       // swap the arguments in the uvw machine call. It gives the correct result on real data
       // although the casacore manual clearly says that the first argument is "out" and the second is "in".
//       machinePtr.reset(new casa::UVWMachine(tangent, phaseCentre, false, true));
       machinePtr.reset(new casa::UVWMachine(phaseCentre, tangent, false, true));
   }
   return *machinePtr;
}

/// @brief a helper method to check whether two directions are matching
/// @details It always return false if the reference frames are different (although
/// the physical direction may be the same). It is aligned with the typical use case as
/// the reference frame is usually the same for all tangent points. If the frames are
/// the same, actual directions are compared. False is returned if the distance between
/// them is more than the tolerance. This non-static method takes the tolerance set in
/// the constructor.
/// @param[in] dir1 first direction
/// @param[in] dir2 second direction
/// @return true, if they are matching
bool UVWMachineCache::compare(const casa::MDirection &dir1, const casa::MDirection &dir2) const
{
  return compare(dir1,dir2,itsTolerance);
}

/// @brief a helper method to check whether two directions are matching
/// @details It always return false if the reference frames are different (although
/// the physical direction may be the same). It is aligned with the typical use case as
/// the reference frame is usually the same for all tangent points. If the frames are
/// the same, actual directions are compared. False is returned if the distance between
/// them is more than the tolerance
/// @param[in] dir1 first direction
/// @param[in] dir2 second direction
/// @param[in] tolerance angle tolerance (in radians)
/// @return true, if they are matching
bool UVWMachineCache::compare(const casa::MDirection &dir1, const casa::MDirection &dir2, const double tolerance)
{
   if (dir1.getRef().getType() != dir2.getRef().getType()) {
       return false;
   }
   return dir1.getValue().separation(dir2.getValue()) < tolerance;
}


/// @brief obtain the index corresponding to a particular tangent point
/// @details If the cache entry needs updating, the appropriate shared pointer will
/// be reset. This method updates itsTangentPoints, if necessary.
/// @param[in] phaseCentre direction to the input phase centre
/// @param[in] tangent direction to tangent point
/// @return cache index
size_t UVWMachineCache::getIndex(const casa::MDirection &phaseCentre, 
                                 const casa::MDirection &tangent) const
{
   // search from the newest element backwards (i.e. most likely the match is the most
   // recently used tangent point)
   for (int pos=0; pos<int(itsTangentPoints.size()); ++pos) {
        int index = int(itsOldestElement)-pos-1;
        if (index<0) {
            // wrap around the end of the vector
            index += int(itsCache.size());
        }
        ASKAPDEBUGASSERT((index>=0) && (index<int(itsTangentPoints.size())));
        if (compare(tangent, itsTangentPoints[index]) && compare(phaseCentre, itsPhaseCentres[index])) {
            return size_t(index);
        }
   }
   // there has been no match, need to replace itsOldestElement
   ASKAPDEBUGASSERT(itsOldestElement<itsCache.size());
   const size_t result = itsOldestElement++;
   // machine needs updating
   itsCache[result].reset(); 
   if (itsOldestElement >= itsCache.size()) {
       // wrap around the end of the vector
       itsOldestElement = 0;
   }
   itsTangentPoints[result] = tangent;
   itsPhaseCentres[result] = phaseCentre;
   return result;
}

