/// @file
/// @brief accessor adapter fitting the best w-plane 
///
/// @details This is an adapter to data accessor which fits a plane
/// into w=w(u,v) and corrects w to represent the distance from
/// this plane rather than the absolute w-term. The planar component
/// can be taken out as a shift in the image space. The adapter provides
/// methods to obtain the magnitude of the shift (i.e. fit coefficients).
/// This class also checkes whether the deviation from the plane is within the
/// tolerance set-up at the construction. A new plane is fitted if necessary. 
/// @note An exception is thrown if the layout is so non-coplanar, that the
/// required tolerance cannot be met.
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

// own includes
#include <dataaccess/BestWPlaneDataAccessor.h>
// we use just a static method to track changes of the tangent point
#include <dataaccess/UVWMachineCache.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief constructor
/// @details The only parameter is the w-term tolerance in wavelengths
/// If the deviation from the fitted plane exceeds the tolerance, a new
/// fit will be performed. If it doesn't help, an exception will be thrown.
///
/// @param[in] tolerance w-term tolerance in wavelengths
/// @note An exception could be thrown during the actual processing, not
/// in the constructor call itself.
BestWPlaneDataAccessor::BestWPlaneDataAccessor(const double tolerance) : itsWTolerance(tolerance),
       itsCoeffA(0.), itsCoeffB(0.), itsUVWChangeMonitor(changeMonitor())
{
}

/// @brief uvw after rotation
/// @details This method subtracts the best plane out of the w coordinates
/// (after uvw-rotation) and return the resulting vectors.
/// @param[in] tangentPoint tangent point to rotate the coordinates to
/// @return uvw after rotation to the new coordinate system for each row
/// @note An exception is thrown if the layout is so non-coplanar that
/// the required tolerance on w-term cannot be met.
const casa::Vector<casa::RigidVector<casa::Double, 3> >&
	         BestWPlaneDataAccessor::rotatedUVW(const casa::MDirection &tangentPoint) const
{
   // original accessor, this would throw an exception if an accessor is not assigned
   const IConstDataAccessor &acc = getROAccessor();
   
   // change monitor should indicate a change for the first ever call to this method
   // (because an associate method should have been called by now)
   if (itsUVWChangeMonitor == changeMonitor()) {
       // just a sanity check to ensure that assumptions hold
       ASKAPCHECK(UVWMachineCache::compare(tangentPoint,itsLastTangentPoint,1e-6),
           "Current implementation implies that only one tangent point is used per single BestWPlaneDataAccessor adapter. "
           "rotatedUVW got tangent point="<<tangentPoint<<", while the last one was "<<itsLastTangentPoint);
       // no change detected, return the buffer
       return itsRotatedUVW;
   }
   // need to compute uvw's
   itsLastTangentPoint = tangentPoint;
   const casa::Vector<casa::RigidVector<casa::Double, 3> >& originalUVW = acc.rotatedUVW(tangentPoint);
   if (itsRotatedUVW.nelements() != originalUVW.nelements()) {
       itsRotatedUVW.resize(originalUVW.nelements());
   }
   
   // tbd
   
   return itsRotatedUVW;
}	         

