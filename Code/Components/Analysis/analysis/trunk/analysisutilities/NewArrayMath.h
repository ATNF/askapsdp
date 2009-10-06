/// @file
///
/// Additional functions to go with those in casa/Arrays/ArrayMath.h
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

#ifndef NEW_ARRAY_MATH_H
#define NEW_ARRAY_MATH_H

#include <iostream>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/namespace.h>

template<class T> inline T madfm(const Array<T> &a){ 
  return madfm (a, False, (a.nelements() <= 100), False); 
}
template<class T> inline T madfm(const Array<T> &a, Bool sorted){ 
  return madfm (a, sorted, (a.nelements() <= 100), False); 
}
template<class T> inline T madfmInPlace(const Array<T> &a, Bool sorted = False){ 
  return madfm (a, sorted, (a.nelements() <= 100), True); 
}
template<class T> T madfm(const Array<T> &a, Bool sorted, Bool takeEvenMean, Bool inPlace = False){ 
  Block<T> tmp; return madfm (a, tmp, sorted, takeEvenMean, inPlace); 
}
template<class T> T madfm(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool takeEvenMean, Bool inPlace = False);

template<class T> inline T sihr(const Array<T> &a){ 
  return sihr (a, False, False); 
}
template<class T> inline T sihr(const Array<T> &a, Bool sorted){ 
  return sihr (a, sorted, False); 
}
template<class T> T sihr(const Array<T> &a, Bool sorted, Bool inPlace = False){ 
  Block<T> tmp; return sihr (a, tmp, sorted, inPlace); 
}
template<class T> T sihr(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool inPlace = False);

#include <analysisutilities/NewArrayMath.tcc>

#endif
