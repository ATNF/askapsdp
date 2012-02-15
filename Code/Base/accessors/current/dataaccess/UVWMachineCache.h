/// @file
/// @brief cache of UVWMachines
/// @details
/// This class maintains the cache of UVW Machines (a pair of tangent point and phase centre directions 
/// is  the key). The number of machines cached and the direction tolerance are specified as parameters.
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

#ifndef UVW_MACHINE_CACHE_H
#define UVW_MACHINE_CACHE_H

// std includes
#include <vector>

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes
#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>


namespace askap {

namespace accessors {

/// @brief cache of UVW Machines
/// @details
/// This class maintains the cache of UVW Machines (a pair of tangent point and phase centre directions 
/// is  the key). The number of machines cached and the direction tolerance are specified as parameters.
/// @ingroup dataaccess
struct UVWMachineCache {
   
   /// @brief UVWMachine class type
   /// @details For debugging, it is handy to substitute UVWMachine class by another type
   /// (to be able to implement only methods we need and to reduce our dependence on 
   /// the fix to casacore). This typedef defines what class is cached.
   typedef casa::UVWMachine  machineType;

   /// @brief construct the cache
   /// @details Set up basic parameters of the cache.
   /// @param[in] cacheSize a number of uvw machines in the cache (default is 1)
   /// @param[in] tolerance pointing direction tolerance in radians, exceeding which leads 
   /// to initialisation of a new UVW Machine
   explicit UVWMachineCache(size_t cacheSize = 1, double tolerance = 1e-6);
   
   /// @brief destructor to print some stats
   /// @details This method writes in the log cache utilisation statistics
   ~UVWMachineCache();
   
   /// @brief obtain machine for a particular tangent point and phase centre
   /// @details This is the main method of the class.
   /// @param[in] phaseCentre direction to the input phase centre
   /// @param[in] tangent direction to tangent point
   /// @return a const reference to uvw machine 
   const machineType& machine(const casa::MDirection &phaseCentre, 
                                   const casa::MDirection &tangent) const;
   
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
   bool compare(const casa::MDirection &dir1, const casa::MDirection &dir2) const;

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
   static bool compare(const casa::MDirection &dir1, const casa::MDirection &dir2, const double tolerance);

protected:
   /// @brief obtain the index corresponding to a particular tangent point
   /// @details If the cache entry needs updating, the appropriate shared pointer will
   /// be reset. This method updates itsTangentPoints, if necessary.
   /// @param[in] phaseCentre direction to the input phase centre
   /// @param[in] tangent direction to tangent point
   /// @return cache index
   size_t getIndex(const casa::MDirection &phaseCentre, const casa::MDirection &tangent) const;
   
private:

   /// @brief the actual cache of uvw machines
   /// @note We're using a plain vector-based cache here instead of the std queue because we
   /// need a flexible iteration over all elements to determine whether the requested tangent
   /// point is already in the cache.
   mutable std::vector<boost::shared_ptr<machineType> > itsCache;

   /// @brief cached tangent directions
   mutable std::vector<casa::MDirection> itsTangentPoints;

   /// @brief cached phase centre directions
   mutable std::vector<casa::MDirection> itsPhaseCentres;
   
   /// @brief index of the oldest element in the cache
   mutable size_t itsOldestElement;
      
   /// @brief direction tolerance
   /// @details It determines whether we a new machine has to be created
   double itsTolerance; 
};

} // namespace accessors

} // namespace askap

#endif // #ifndef UVW_MACHINE_CACHE_H

