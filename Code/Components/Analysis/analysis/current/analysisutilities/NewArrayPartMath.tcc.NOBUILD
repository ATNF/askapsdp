/// @file
///
/// Additional functions to go with those in casa/Arrays/ArrayPartMath.tcc
///
/// @copyright (c) 2008 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///

#include <iostream>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/namespace.h>
#include <analysisutilities/NewArrayMath.h>
#include <analysisutilities/NewArrayPartMath.h>


template<class T> Array<T> partialMadfms (const Array<T>& array,
					  const IPosition& collapseAxes,
					  Bool takeEvenMean,
					  Bool inPlace)
{
  // Need to make shallow copy because operator() is non-const.
  Array<T> arr = array;
  // Is there anything to collapse?
  if (collapseAxes.nelements() == 0) {
    return (inPlace  ?  array : array.copy());
  }
  const IPosition& shape = array.shape();
  uInt ndim = shape.nelements();
  if (ndim == 0) {
    return Array<T>();
  }
  // Get the remaining axes.
  // It also checks if axes are specified correctly.
  IPosition resAxes = IPosition::otherAxes (ndim, collapseAxes);
  uInt ndimRes = resAxes.nelements();
  // Create the result shape.
  // Create blc and trc to step through the input array.
  IPosition resShape(ndimRes);
  IPosition blc(ndim, 0);
  IPosition trc(shape-1);
  for (uInt i=0; i<ndimRes; ++i) {
    resShape[i] = shape[resAxes[i]];
    trc[resAxes[i]] = 0;
  }
  if (ndimRes == 0) {
    resShape.resize(1);
    resShape[0] = 1;
  }
  Array<T> result (resShape);
  Bool deleteRes;
  T* resData = result.getStorage (deleteRes);
  T* res = resData;
  Block<T> tmp;
  // Loop through all data and assemble as needed.
  IPosition pos(ndimRes, 0);
  while (True) {
    *res++ = madfm(arr(blc,trc), tmp, False, takeEvenMean, inPlace);
    uInt ax;
    for (ax=0; ax<ndimRes; ax++) {
      if (++pos(ax) < resShape(ax)) {
	blc[resAxes[ax]]++;
	trc[resAxes[ax]]++;
	break;
      }
      pos(ax) = 0;
      blc[resAxes[ax]] = 0;
      trc[resAxes[ax]] = 0;
    }
    if (ax == ndimRes) {
      break;
    }
  }
  result.putStorage (resData, deleteRes);
  return result;
}
