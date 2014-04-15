/// @file
/// 
/// @brief helper class to unwrap the phase 
/// @details This class attempts to unwrap the phase using a simple threshold and the state of the previous phase. 
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

#ifndef ASKAP_SCIMATH_UTILS_PHASE_UNWRAPPER_H
#define ASKAP_SCIMATH_UTILS_PHASE_UNWRAPPER_H

#include <casa/BasicSL/Constants.h>


namespace askap {

namespace scimath {

/// @brief helper class to unwrap the phase 
/// @details This class attempts to unwrap the phase using a simple threshold and the state of the previous phase. 
/// @ingroup utils
template<typename T>
class PhaseUnwrapper {
public:

   /// @brief construct the object with the given tolerance
   /// @param[in] tolerance optional tolerance in radians
   explicit PhaseUnwrapper(const T tolerance = static_cast<T>(3 * casa::C::pi / 2)) : itsTolerance(tolerance), 
            itsWrapCompensation(static_cast<T>(0.)), itsDataSighted(false) {}
            
   /// @brief process phase
   /// @details Process one phase point, unwrap if necessary
   /// @param[in] phase input phase in radians
   T operator()(const T phase);
   
private:
   
   /// @brief tolerance to trigger unwrapping
   T itsTolerance;
   
   /// @brief wrap compensation
   T itsWrapCompensation;
   
   /// @brief true, if some data have already been processed
   bool itsDataSighted;
   
   /// @brief previous unmodified phase in radians
   T itsPrevOrigPhase;
        
}; // class PhaseUnwrapper

} // namespace scimath

} // namespace askap

#include "utils/PhaseUnwrapper.tcc"

#endif // #ifndef ASKAP_SCIMATH_UTILS_PHASE_UNWRAPPER_H


