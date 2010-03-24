/// @file
///
/// @brief Gridder taking w-term into account
/// @details
/// This is a base class for all gridders taking w-term into account. It manages sampling
/// in w-space (which may be non-linear, if so chosen by the user)
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

#include <gridding/SphFuncVisGridder.h>

#ifndef W_DEPENDENT_GRIDDER_BASE_H
#define W_DEPENDENT_GRIDDER_BASE_H

namespace askap {

namespace synthesis {

/// @brief Gridder taking w-term into account
/// @details
/// This is a base class for all gridders taking w-term into account. It manages sampling
/// in w-space (which may be non-linear, if so chosen by the user)
/// @ingroup gridding
class WDependentGridderBase : public SphFuncVisGridder 
{
public:
   /// @brief constructor initialising for default linear sampling   
   /// @details
   /// @param[in] wmax Maximum baseline (wavelengths)
   /// @param[in] nwplanes Number of w planes   
   WDependentGridderBase(const double wmax, const int nwplanes); 
   
   /// @brief obtain the number of w-planes
   /// @details 
   /// @return the number of w-planes
   int nWPlanes() const { return itsNWPlanes;}
   
   /// @brief obtain plane number
   /// @details
   /// @param[in] w w-term (in wavelengths) to map
   /// @return plane number
   /// @note an exception is thrown if the requested w-term lies outside (-wmax,wmax) range
   int getWPlane(const double w) const;
   
   /// @brief obtain w-term for a given plane
   /// @details This is a reverse operation to wPlaneNumber.
   /// @param[in] plane plane number
   /// @return w-term (in wavelengths) corresponding to the given plane
   /// @note an exception is thrown (in debug mode) if the plane is outside [0.plane) range
   double getWTerm(const int plane) const;
   
private:
   /// Scaling
   double itsWScale;
   /// Number of w planes
   int itsNWPlanes;
};

} // namespace synthesis

} // namespace askap


#endif // #ifndef W_DEPENDENT_GRIDDER_BASE_H


