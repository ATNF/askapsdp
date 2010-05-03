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

namespace scimath {

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

/// @brief Extract a centered subarray, which is a given factor smaller
/// @details Most padding applications in the ASKAPsoft require operations on just two
/// axes. This method uses centeredSubArray to extract an array which is a padding times
/// smaller on the first two axes. Other axes are not altered. The subarray and the original
/// array have the same centre.
/// @param[in] source input array
/// @param[in] padding padding factor (should be a positive number)
/// @return extracted subarray
template<typename T>
casa::Array<T> PaddingUtils::extract(casa::Array<T> &source, const float padding)
{
   return centeredSubArray(source, unpadShape(source.shape(),padding));
}

/// @brief clip outer edges
/// @details To make padding effective we need to fill the outer edges with zeros after non-linear
/// operations such as preconditioning. This method leaves the inner subarray of the given 2D shape 
/// intact and fills the rest of the array with zeros. Any type is supported which allows assignment of
/// a float (0.).
/// @param[in] source array to modify
/// @param[in] size shape of the inner subarray to be left intact
/// @note At the moment, clipping can only happen on the first two axes and the inner subarray must be 
/// two-dimensional
template<typename T>
void PaddingUtils::clip(casa::Array<T> & source, const casa::IPosition &size)
{
   const casa::IPosition shape = source.shape();
   ASKAPDEBUGASSERT(shape.nelements()>=2);
   ASKAPASSERT(size.nelements() >= 2);
   casa::IPosition end(shape);
   for (uint index=0;index<end.nelements();++index) {
        ASKAPDEBUGASSERT(end[index]>=1);
        end[index]--;
   }

       if (shape[0]>size[0]+1) {
           // need clipping along the first axis
           casa::IPosition start(shape.nelements(),0);
           end[0] = (shape[0]-size[0])/2-1;
           end[1] = shape[1]-1; // although this step is strictly speaking unnecessary
           source(start,end).set(0.); 
           
           end[0] = shape[0]-1;
           start[0] = (shape[0]+size[0])/2;
           source(start,end).set(0.);
       }
       
       if (shape[1]>size[1]+1) {
           // need clipping along the second axis
           casa::IPosition start(shape.nelements(),0);
           start[0]=(shape[0]-size[0])/2;
           end[0]=(shape[0]+size[0])/2;
           if (start[0]<0) {
               start[0] = 0;
           }
           if (end[0]+1 > shape[0]) {
               end[0] = shape[0] - 1;
           }
           start[1] = 0;
           end[1] = (shape[1]-size[1])/2-1;
           source(start,end).set(0.);
           
           start[1] = (shape[1]+size[1])/2;
           end[1] = shape[1]-1;
           source(start,end).set(0.);
       }       
}


} // namespace scimath

} // namespace askap



#endif //#ifndef PADDING_UTILS_TCC


