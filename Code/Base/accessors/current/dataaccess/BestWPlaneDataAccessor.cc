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
using namespace askap::accessors;

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
   
   // compute tolerance in metres to match units of originalUVW
   const casa::Vector<double>& freq = acc.frequency();
   ASKAPCHECK(freq.nelements()>=1, "An unexpected accessor with zero spectral channels has been encountered");
   
   // use the largest frequency/smallest wavelength, i.e. worst case scenario
   const double maxFreq = freq.nelements() == 1 ? freq[0] : casa::max(freq[0],freq[freq.nelements()-1]);
   ASKAPDEBUGASSERT(maxFreq > 0.); 
   const double tolInMetres = itsWTolerance * casa::C::c / maxFreq;
   
   const double maxDeviation = updatePlaneIfNecessary(originalUVW, tolInMetres);
   
   ASKAPCHECK(maxDeviation < tolInMetres, "The antenna layout is significantly non-coplanar. "
             "The largest w-term deviation after the fit of "<<maxDeviation<<" metres exceedes the w-term tolerance of "<<
              itsWTolerance<<" wavelengths equivalent to "<<tolInMetres<<" metres.");
   for (casa::uInt row=0; row<originalUVW.nelements(); ++row) {
        const casa::RigidVector<casa::Double, 3> currentUVW = originalUVW[row];
        itsRotatedUVW[row] = currentUVW;
        // subtract the current plane
        itsRotatedUVW[row](2) -= coeffA()*currentUVW(0) + coeffB()*currentUVW(1);
   }
   
   return itsRotatedUVW;
}	         

/// @brief calculate the largest deviation from the current fitted plane
/// @details This helper method iterates through the given uvw's and returns
/// the largest deviation of the w-term from the current best fit plane.
/// @param[in] uvw a vector with uvw's
/// @return the largest w-term deviation from the current plane (same units as uvw's)
double BestWPlaneDataAccessor::maxWDeviation(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw) const
{
   double maxDeviation = 0.;

   // we fit w=Au+Bv, the following lines compute the largest deviation from the current plane.

   for (casa::uInt row=0; row<uvw.nelements(); ++row) {
        const casa::RigidVector<casa::Double, 3> currentUVW = uvw[row];
        const double deviation = fabs(coeffA()*currentUVW(0) + coeffB()*currentUVW(1) - currentUVW(2));
        if (deviation > maxDeviation) {
            maxDeviation = deviation;
        }
   }
   
   return maxDeviation;
}

/// @brief fit a new plane and update coefficients if necessary
/// @details This method iterates over given uvw's, checks whether the 
/// largest deviation of the w-term from the current plane is above the 
/// tolerance and updates the fit coefficients if it is. 
/// planeChangeMonitor() can be used to detect the change in the fit plane.
/// 
/// @param[in] uvw a vector with uvw's
/// @param[in] tolerance tolerance in the same units as uvw's
/// @return the largest w-term deviation from the fitted plane (same units as uvw's)
/// @note If a new fit is performed, the devitation is reported with respect to the
/// new fit (it takes place if the deviation from initial plane exceeds the given tolerance).
/// Therefore, if the returned deviation exceeds the tolerance, the layout is significantly
/// non-coplanar, so the required tolerance cannot be achieved.
/// This method has a conceptual constness as it doesn't change the original accessor.
double BestWPlaneDataAccessor::updatePlaneIfNecessary(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                 double tolerance) const
{
   const double maxDeviation = maxWDeviation(uvw);
      
   // we need at least two rows for a successful fitting, don't bother doing anything if the
   // number of rows is too small or the deviation is below the tolerance
   if ((uvw.nelements() < 2) || (maxDeviation < tolerance)) {
       return maxDeviation;
   }
   
   // we fit w=Au+Bv, the following lines accumulate the necessary sums of the LSF problem
   
   double su2 = 0.; // sum of u-squared
   double sv2 = 0.; // sum of v-squared
   double suv = 0.; // sum of uv-products
   double suw = 0.; // sum of uw-products
   double svw = 0.; // sum of vw-products
   
   for (casa::uInt row=0; row<uvw.nelements(); ++row) {
        const casa::RigidVector<casa::Double, 3> currentUVW = uvw[row];

        su2 += casa::square(currentUVW(0));
        sv2 += casa::square(currentUVW(1));
        suv += currentUVW(0) * currentUVW(1);
        suw += currentUVW(0) * currentUVW(2);
        svw += currentUVW(1) * currentUVW(2);                
   }
   
   // we need a non-zero determinant for a successful fitting
   // some tolerance has to be put on the determinant to avoid unconstrained fits
   // we just accept the current fit results if the new fit is not possible
   const double D = su2 * sv2 - casa::square(suv);

   if (fabs(D) < 1e-7) {
       return maxDeviation;
   }

   // make an update to the coefficients
   itsCoeffA = (sv2 * suw - suv * svw) / D;
   itsCoeffB = (su2 * svw - suv * suw) / D;
   itsPlaneChangeMonitor.notifyOfChanges();
   return maxWDeviation(uvw);
}

