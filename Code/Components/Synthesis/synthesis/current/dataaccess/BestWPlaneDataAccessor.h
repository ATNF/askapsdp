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

#ifndef BEST_W_PLANE_DATA_ACCESSOR_H
#define BEST_W_PLANE_DATA_ACCESSOR_H

// own includes
#include <dataaccess/DataAccessorAdapter.h>

namespace askap {

namespace synthesis {

/// @brief adapter accessor fitting the best w-plane 
/// @details This is an adapter to data accessor which fits a plane
/// into w=w(u,v) and corrects w to represent the distance from
/// this plane rather than the absolute w-term. The planar component
/// can be taken out as a shift in the image space. The adapter provides
/// methods to obtain the magnitude of the shift (i.e. fit coefficients).
/// This class also checkes whether the deviation from the plane is within the
/// tolerance set-up at the construction. A new plane is fitted if necessary. 
/// @note An exception is thrown if the layout is so non-coplanar, that the
/// required tolerance cannot be met.
/// @ingroup dataaccess
class BestWPlaneDataAccessor : public DataAccessorAdapter {
public:
   /// @brief constructor
   /// @details The only parameter is the w-term tolerance in wavelengths
   /// If the deviation from the fitted plane exceeds the tolerance, a new
   /// fit will be performed. If it doesn't help, an exception will be thrown.
   ///
   /// @param[in] tolerance w-term tolerance in wavelengths
   /// @note An exception could be thrown during the actual processing, not
   /// in the constructor call itself.
   explicit BestWPlaneDataAccessor(const double tolerance);
   
   /// @brief uvw after rotation
   /// @details This method subtracts the best plane out of the w coordinates
   /// (after uvw-rotation) and return the resulting vectors.
   /// @param[in] tangentPoint tangent point to rotate the coordinates to
   /// @return uvw after rotation to the new coordinate system for each row
   /// @note An exception is thrown if the layout is so non-coplanar that
   /// the required tolerance on w-term cannot be met.
   virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&
	         rotatedUVW(const casa::MDirection &tangentPoint) const;
   
   // fitted plane parameters
   
   /// @brief obtain fit coefficient A
   /// @details We fit w=Au+Bv, this method returns the coefficient A
   /// @return fit coefficient A
   inline double coeffA() const { return itsCoeffA; }

   /// @brief obtain fit coefficient B
   /// @details We fit w=Au+Bv, this method returns the coefficient B
   /// @return fit coefficient B
   inline double coeffB() const { return itsCoeffB; }
   
private:
   /// @brief w-term tolerance in wavelengths
   /// @details If the deviation from the fitted plane exceeds the tolerance, a new
   /// fit will be performed. If it doesn't help, an exception will be thrown.
   double itsWTolerance;
   
   /// @brief fit parameter A
   /// @details We fit w=Au+Bv, this is the coefficient A
   double itsCoeffA;
   
   /// @brief fit parameter B
   /// @details We fit w=Au+Bv, this is the coefficient B
   double itsCoeffB;
   
   /// @brief change monitor to manage caching
   /// @details This change monitor is updated every time
   /// a new uvw's are calculated (and therefore the quality of
   /// the fit is checked and a new fit is done if necessary).
   scimath::ChangeMonitor itsUVWChangeMonitor;
   
   /// @brief buffer for rotated UVW vector with corrected w
   mutable casa::Vector<casa::RigidVector<casa::Double, 3> > itsRotatedUVW;   
   
   /// @brief last tangent point
   /// @details This field is added just to be able to do extra checks
   /// that tanget point is stays fixed for the same accessor. If it could
   /// change, we would need a more intelligent caching of itsRotatedUVW
   /// because uvw-rotation is tangent point-dependent.
   mutable casa::MDirection itsLastTangentPoint;    
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef BEST_W_PLANE_DATA_ACCESSOR_H


