/// @file
/// @brief all logic behind uvw rotations and associated delays
/// @details
/// This class does all operations with UVW Machines to handle rotations and associated delays.
/// It is inherited from UVWMachineCache and uses it to maintain the cache. This class is
/// intended to manage accessor fields corresponding to uvw rotation as the functionality of
/// the CachedAccessorField template is not sufficient (can't pass parameters, which affect caching)
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

#ifndef UVW_ROTATION_HANDLER_H
#define UVW_ROTATION_HANDLER_H

#include <dataaccess/UVWMachineCache.h>
#include <dataaccess/IConstDataAccessor.h>
#include <measures/Measures/MDirection.h>

namespace askap {

namespace accessors {

/// @brief all logic behind uvw rotations and associated delays
/// @details
/// This class does all operations with UVW Machines to handle rotations and associated delays.
/// It is inherited from UVWMachineCache and uses it to maintain the cache. This class is
/// intended to manage accessor fields corresponding to uvw rotation as the functionality of
/// the CachedAccessorField template is not sufficient (can't pass parameters, which affect caching)
/// @ingroup dataaccess
struct UVWRotationHandler : protected UVWMachineCache {

   /// @brief construct the handler
   /// @details Set up basic parameters of the underlying machine cache.
   /// @param[in] cacheSize a number of uvw machines in the cache (default is 1)
   /// @param[in] tolerance pointing direction tolerance in radians, exceeding which leads 
   /// to initialisation of a new UVW Machine and recompute of the rotated uvws/delays
   explicit UVWRotationHandler(size_t cacheSize = 1, double tolerance = 1e-6);

   /// @brief invalidate the cache
   /// @details A call to this method invalidates the cache (for each accessor row) of rotated
   /// uvws and delays. Nothing is done for uvw machines as UVWMachineCache takes care of this.
   /// This method is const as effectively non-const operations are only for caching purposes.
   void invalidate() const;
   
   /// @brief obtain rotated uvws
   /// @details
   /// Use parameters in the given accessor to compute rotated uvws
   /// @param[in] acc const reference to the input accessor (need phase centre info, uvw, etc)
   /// @param[in] tangent direction to the tangent point
   /// @return const reference to rotated uvws
   /// @note the method doesn't monitor a change to the accessor. It expects that invalidate 
   /// is called explicitly when recalculation is needed (i.e. iterator moved to the next iteration, etc)
   const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw(const IConstDataAccessor &acc, 
               const casa::MDirection &tangent) const;

   /// @brief obtain delays corresponding to rotation
   /// @details
   /// Use parameters in the given accessor to compute delays. This method calls rotatedUVWs and does
   /// some extra job on the delays if tangent != imageCentre
   /// @param[in] acc const reference to the input accessor (need phase centre info, uvw, etc)
   /// @param[in] tangent direction to the tangent point
   /// @param[in] imageCentre direction to the image centre
   /// @return const reference to delay vector
   /// @note the method doesn't monitor a change to the accessor. It expects that invalidate 
   /// is called explicitly when recalculation is needed (i.e. iterator moved to the next iteration, etc)
   const casa::Vector<casa::Double>& delays(const IConstDataAccessor &acc, 
               const casa::MDirection &tangent, const casa::MDirection &imageCentre) const;
                  
private:
   /// @brief rotated uvw coordinates
   mutable casa::Vector<casa::RigidVector<casa::Double, 3> > itsRotatedUVWs;
   
   /// @brief internal buffer for delay associated with uvw rotation
   mutable casa::Vector<casa::Double> itsDelays;
   
   /// @brief flag that rotated uvws and delays are up to date
   /// @details If this field is uptodate itsDelays contain some valid information too.
   mutable bool itsValid;

   /// @brief tangnet point for which this cache is valid
   /// @details
   /// Rotation depends on the tangent point. Cache of UVW Machines returns the appropriate machine,
   /// but we also have to recompute the cache of results of it changes.
   mutable casa::MDirection itsTangentPoint;
   
   /// @brief current image centre used to calculate delays
   /// @details
   /// If the image centre changes (and is different from tangent point), an additional translation
   /// in the tangent plane is needed for the faceting to work. This is equivalent to uvw-dependent
   /// delay. This method adds an extra delay if necessary. Theoretically, the image centre can
   /// be changed any number of times without recomputing delays. Although if the number of changes
   /// is too large some round-off errors may accumulate as we just add some extra delay to the
   /// cache following every change to this field.
   mutable casa::MDirection itsImageCentre;
   
};


} // namespace accessors

} // namespace askap

#endif // UVW_ROTATION_HANDLER_H
