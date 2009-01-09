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

#ifndef PADDING_UTILS_TCC
#define PADDING_UTILS_TCC

#include <askap/AskapError.h>
#include <casa/Arrays/Array.h>
namespace askap {

namespace synthesis {

/// @brief Extract a centered subarray of a given shape
/// @details This helper method is used for faceted imaging with padding (and overlap) of facets.
/// It extracts a subarray of a given shape from the centre of the given array.
/// @param[in] source source array
/// @param[in] shape required shape
/// @return extracted subarray
template<typename T>
casa::Array<T> PaddingUtils::centeredSubArray(casa::Array<T> &source, 
                                                   const casa::IPosition &shape)
{
  const casa::IPosition srcShape = source.shape();
  ASKAPDEBUGASSERT(shape.nelements() == srcShape.nelements());
  casa::IPosition blc(shape), trc(shape);
  for (size_t i=0;i<blc.nelements();++i) {
       blc[i] = (srcShape[i]-shape[i])/2;
       ASKAPCHECK(blc[i]>=0, "A bigger array is requested from centeredSubArray, dimension "<<i<<
                 " inputSize="<<srcShape[i]<<" outputSize="<<shape[i]);
       ASKAPDEBUGASSERT(srcShape[i]>0);
       ASKAPDEBUGASSERT(shape[i]>0);          
       trc[i] = (srcShape[i]+shape[i])/2-1; 
       
       ASKAPDEBUGASSERT(trc[i]-blc[i]+1 == shape[i]);
  }
  return source(blc,trc);
}


} // namespace synthesis

} // namespace askap



#endif //#ifndef PADDING_UTILS_TCC


