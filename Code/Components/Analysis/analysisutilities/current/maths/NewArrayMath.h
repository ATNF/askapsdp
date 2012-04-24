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

/// @brief Functions to return the median deviation from the median
/// for an array of data
/// @details The functions can take a variety of possible
/// parameters. The following is taken from the casacore information on
/// the median() function:
///
/// The median of "a" is a(n/2).
/// If a has an even number of elements and the switch takeEvenMean is set,
/// the median is 0.5*(a(n/2) + a((n+1)/2)).
/// According to Numerical Recipes (2nd edition) it makes little sense to take
/// the mean if the array is large enough (> 100 elements). Therefore
/// the default for takeEvenMean is False if the array has > 100 elements,
/// otherwise it is True.
/// <br>If "sorted"==True we assume the data is already sorted and we
/// compute the median directly. Otherwise the function GenSort::kthLargest
/// is used to find the median (kthLargest is about 6 times faster
/// than a full quicksort).
/// <br>Finding the median means that the array has to be (partially)
/// sorted. By default a copy will be made, but if "inPlace" is in effect,
/// the data themselves will be sorted. That should only be used if the
/// data are used not thereafter.
/// @name
/// @{
template<class T> inline T madfm(const Array<T> &a)
{
    return madfm(a, False, (a.nelements() <= 100), False);
}
template<class T> inline T madfm(const Array<T> &a, Bool sorted)
{
    return madfm(a, sorted, (a.nelements() <= 100), False);
}
template<class T> inline T madfmInPlace(const Array<T> &a, Bool sorted = False)
{
    return madfm(a, sorted, (a.nelements() <= 100), True);
}
template<class T> T madfm(const Array<T> &a, Bool sorted, Bool takeEvenMean, Bool inPlace = False)
{
    Block<T> tmp; return madfm(a, tmp, sorted, takeEvenMean, inPlace);
}
template<class T> T madfm(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool takeEvenMean, Bool inPlace = False);
/// @}

/// @brief Functions to return the semi-interhexile range for an array
/// of data
/// @details The functions can take a variety of possible
/// parameters. The following is taken from the casacore information on
/// the median() function:
///
/// If "sorted"==True we assume the data is already sorted and we
/// compute the SIHR directly. Otherwise the function GenSort::kthLargest
/// is used to find the SIHR (kthLargest is about 6 times faster
/// than a full quicksort).
/// <br>Finding the median means that the array has to be (partially)
/// sorted. By default a copy will be made, but if "inPlace" is in effect,
/// the data themselves will be sorted. That should only be used if the
/// data are used not thereafter.
/// @name
/// @{
template<class T> inline T sihr(const Array<T> &a)
{
    return sihr(a, False, False);
}
template<class T> inline T sihr(const Array<T> &a, Bool sorted)
{
    return sihr(a, sorted, False);
}
template<class T> T sihr(const Array<T> &a, Bool sorted, Bool inPlace = False)
{
    Block<T> tmp; return sihr(a, tmp, sorted, inPlace);
}
template<class T> T sihr(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool inPlace = False);
/// @}

#include <maths/NewArrayMath.tcc>

#endif
