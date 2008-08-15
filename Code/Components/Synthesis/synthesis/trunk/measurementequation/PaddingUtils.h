/// @file
///
/// PaddingUtils a class containing utilities used for FFT padding in preconditioners. Code like this
/// can probably be moved to a higer level. At this stage we just need to make these methods available not
/// just to the WienerPreconditioner, but for other classes as well.
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

#ifndef PADDING_UTILS_H
#define PADDING_UTILS_H

#include <lattices/Lattices/Lattice.h>

namespace askap {

namespace synthesis {

/// PaddingUtils a class containing utilities used for FFT padding in preconditioners. Code like this
/// can probably be moved to a higer level. At this stage we just need to make these methods available not
/// just to the WienerPreconditioner, but for other classes as well.
/// @ingroup measurementequation
struct PaddingUtils {
  /// @brief Inject source into the centre quarter of the target
  /// @details 
  /// @param[in] target target array to alter, the source will be converted to Complex and stored in the 
  /// inner quarter of the target
  /// @param[in] source input array
  static void inject(casa::Lattice<casa::Complex>& target, casa::Lattice<float>& source);
      
  /// @brief Extract target from the center quarter of the source 
  /// @details
  /// @param[in] target target array to save the reslt, a real part of the inner quarter of the the source array 
  /// will be extracted
  /// @param[in] source input array
  static void extract(casa::Lattice<float>& target, casa::Lattice<casa::Complex>& source);  
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef PADDING_UTILS_H
