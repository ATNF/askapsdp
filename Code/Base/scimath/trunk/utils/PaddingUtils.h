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
#include <casa/Arrays/Array.h>

namespace askap {

namespace scimath {

/// PaddingUtils a class containing utilities used for FFT padding in preconditioners. Code like this
/// can probably be moved to a higer level. At this stage we just need to make these methods available not
/// just to the WienerPreconditioner, but for other classes as well.
/// @ingroup utils
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
  
  /// @brief Extract a centered subarray of a given shape
  /// @details This helper method is used for faceted imaging with padding (and overlap) of facets.
  /// It extracts a subarray of a given shape from the centre of the given array.
  /// @param[in] source source array
  /// @param[in] shape required shape
  /// @return extracted subarray
  template<typename T>
  static casa::Array<T> centeredSubArray(casa::Array<T> &source, const casa::IPosition &shape);  
  
  /// @brief Extract a centered subarray, which is a given factor smaller
  /// @details Most padding applications in the ASKAPsoft require operations on just two
  /// axes. This method uses centeredSubArray to extract an array which is a padding times
  /// smaller on the first two axes. Other axes are not altered. The subarray and the original
  /// array have the same centre.
  /// @param[in] source input array
  /// @param[in] padding padding factor (should be a positive number)
  /// @return extracted subarray
  template<typename T>
  static casa::Array<T> extract(casa::Array<T> &source, const float padding);
  
  /// @brief helper method to get padded shape
  /// @details Most padding applications in the ASKAPsoft require operations on just two
  /// axes. This method froms a shape of an array padded on first two axes with the given factor.
  /// @param[in] shape shape of the original array
  /// @param[in] padding padding factor
  /// @return shape of the padded array
  static casa::IPosition paddedShape(const casa::IPosition &shape, const float padding);
  
  /// @brief padding with fft
  /// @details Sometimes it is necessary to do padding in the other domain. This routine 
  /// does the Fourier transform, pad the result to the size of the output and then transforms 
  /// back to the original domain. It is done if the size of the output array along the first two
  /// axes is larger than the size of the input array. If the output array size is smaller, just 
  /// the inner subimage is copied and no fft is done. Equal size result in no operation.
  /// @note Both input and output arrays should be at least 2-dimensional, otherwise an exception
  /// is thrown (in debug mode). 
  /// @param[in] in input array 
  /// @param[in] out output array (should already be resized to a desired size) 
  static void fftPad(const casa::Array<double>& in, casa::Array<double>& out);
  
  /// @brief padding with fft
  /// @details This variant of the method is intended for the case where internal padding is used.
  /// The input array is padded with fft to a larger area (size is controlled by the given factor)
  /// and then an inner sub-array is extracted. With factor==1, there is no difference from the
  /// other fftPad methid
  /// @note Both input and output arrays should be at least 2-dimensional, otherwise an exception
  /// is thrown (in debug mode). 
  /// @param[in] in input array 
  /// @param[in] out output array (should already be resized to a desired size) 
  /// @param[in] factor additional padding of the output array
  static void fftPad(const casa::Array<double>& in, casa::Array<double>& out, int factor);
  
};


} // namespace scimath

} // namespace askap

#include <utils/PaddingUtils.tcc>

#endif // #ifndef PADDING_UTILS_H
