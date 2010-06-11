/// @file
/// 
/// @brief helper iterator class to assist with spectral line and polarisation images
/// @details Images are represented as array-valued parameters. Constituents of
/// the normal equations are just single-dimension vectors. The images may actually
/// be hypercubes (polarisation and spectral dimensions). This class facilitates
/// iterations over such images (plane by plane).
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

#ifndef MULTI_DIM_ARRAY_PLANE_ITER_TCC
#define MULTI_DIM_ARRAY_PLANE_ITER_TCC

#include <askap/AskapError.h>


namespace askap {

namespace scimath {

/// @brief extract the first 2D plane of a multi-dimensional cube
/// @details This is a static helper method, which can probably go somewhere else as
/// it doesn't conceptionally belong to this method. It does the same thing as taking 
/// the first iteration slice from an array.
/// @param[in] in input array
/// @return output array (single plane)
template<typename T>
casa::Array<T> MultiDimArrayPlaneIter::getFirstPlane(casa::Array<T> &in) {
    const MultiDimArrayPlaneIter iter(in.shape());
    return iter.getPlane(in);
}
   
/// @brief extract a single plane from an array
/// @details This method forms a slice of the given array to extract a single plane corresponding
/// to the current position of the iterator
/// @param[in] in input array
/// @return output array (single plane)
template<typename T>
casa::Array<T> MultiDimArrayPlaneIter::getPlane(casa::Array<T> &in) const
{
  // we may need to add more functionality to this method to take care of situations
  // when the PSF is defined for a single polarisation/channel only
  const casa::IPosition blc(position());
  casa::IPosition trc(blc);
  trc += itsPlaneShape;
  for (casa::uInt dim = 0; dim<trc.nelements(); ++dim) {
       trc[dim] -= 1;
       ASKAPDEBUGASSERT(trc[dim]<itsShape[dim]);
  }
  return in(blc,trc);
}

/// @brief extract a single plane form a 1D array
/// @details This method extracts a single slice from an array flattened to a 1D vector. The slice 
/// corresponds to the current position of the iterator. This method preserves the degenerate
/// dimensions.
/// @param[in] in input vector
/// @return output array (single plane)
template<typename T>
casa::Array<T> MultiDimArrayPlaneIter::getPlane(casa::Vector<T> &in) const
{
  ASKAPDEBUGASSERT(itsShape.product() == in.shape().product()); 
  casa::Array<T> reformedReference = in.reform(itsShape);
  return getPlane(reformedReference);
}


/// @brief extract a single plane into a flattened vector
/// @details This method extracts a single plane slice from an array flattened to a 1D vector. 
/// The slice corresponds to the current position of the iterator. The result is returned as a
/// flattened vector.
/// @param[in] in input vector
/// @return output vector (single plane)
template<typename T>
casa::Vector<T> MultiDimArrayPlaneIter::getPlaneVector(casa::Vector<T> &in) const
{
  casa::Array<T> plane = getPlane(in);
  return plane.reform(casa::IPosition(1,plane.nelements())); 
}
   
/// @brief extract a single plane into a flattened vector
/// @details This method extracts a single plane slice from an array. 
/// The slice corresponds to the current position of the iterator. Unlike getPlane, the result 
/// is returned as a flattened vector.
/// @param[in] in input vector
/// @return output vector (single plane)
template<typename T>
casa::Vector<T> MultiDimArrayPlaneIter::getPlaneVector(casa::Array<T> &in) const
{
  casa::Array<T> plane = getPlane(in);
  return plane.reform(casa::IPosition(1,plane.nelements())); 
}


} // namespace scimath

} // namespace askap

#endif // #ifndef MULTI_DIM_ARRAY_PLANE_ITER_TCC

