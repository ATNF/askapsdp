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

#ifndef W_DEPENDENT_GRIDDER_BASE_H
#define W_DEPENDENT_GRIDDER_BASE_H

#include <gridding/SphFuncVisGridder.h>
#include <gridding/IWSampling.h>

#include <boost/shared_ptr.hpp>

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
   inline int nWPlanes() const { return itsNWPlanes;}
   
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

protected:
   /// @brief obtain wmax
   /// @details This is a helper method to obtain maximum w-term (corresponding to extreme w-planes).
   /// We don't store the original wmax passed in the constructor. Instead, it is recalculated from
   /// scale and number of planes. This also allows to cap it if number of planes is 1 (and so the
   /// result doesn't really depend on the w-plane number as the corresponding w-term is always 0).
   /// This method is only used for non-linear sampling, otherwise scale and number of planes are 
   /// sufficient.
   /// @return maximum w-term in wavelengths
   inline double getWMax() const { return itsWScale * ((itsNWPlanes-1)/2); }
   
private:
   /// Scaling
   double itsWScale;
   /// Number of w planes
   int itsNWPlanes;   
   /// @brief shared pointer to w-sampling helper
   /// @details We use helper classes to implement an arbitrary non-linear sampling in the w-space.
   /// Such a class just maps [-1,1] to [-1,1] taking into account the desired curvature. Implementing
   /// non-linear sampling this way allows us to specify the tranform using meaningful parameters such as
   /// the maximum w-term or number of planes covering 50% of the w-term range. An empty shared pointer 
   /// means that the linear sampling is used. The state of this helper class depends only on the 
   /// actual mapping and is not changed after construction. Therefore, several gridders may reuse the same
   /// mapper class instance and no cloning operation is needed. 
   boost::shared_ptr<IWSampling> itsWSampling;
};

} // namespace synthesis

} // namespace askap


#endif // #ifndef W_DEPENDENT_GRIDDER_BASE_H


