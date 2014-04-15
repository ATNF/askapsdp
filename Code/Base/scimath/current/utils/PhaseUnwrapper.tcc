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

#ifndef ASKAP_SCIMATH_UTILS_PHASE_UNWRAPPER_TCC
#define ASKAP_SCIMATH_UTILS_PHASE_UNWRAPPER_TCC

namespace askap {

namespace scimath {

/// @brief process phase
/// @details Process one phase point, unwrap if necessary
/// @param[in] phase input phase in radians
template<typename T>
T PhaseUnwrapper<T>::operator()(const T phase)
{
  if (itsDataSighted) {
      const T diff = phase - itsPrevOrigPhase;
      if (diff >= itsTolerance) {
          itsWrapCompensation -= 2. * casa::C::pi;
      } else if (diff <= -itsTolerance) {
          itsWrapCompensation += 2. * casa::C::pi;
      }
      itsPrevOrigPhase = phase;
      return phase + itsWrapCompensation;      
  }
  itsPrevOrigPhase = phase;
  itsDataSighted = true;
  return phase;
}



} // namespace scimath

} // namespace askap


#endif // #ifndef ASKAP_SCIMATH_UTILS_PHASE_UNWRAPPER_TCC


