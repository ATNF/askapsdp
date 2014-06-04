/// @file
///
/// Additional functions to go with those in casa/Arrays/ArrayPartMath.h
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

#ifndef NEW_MASK_ARRAY_MATH_H
#define NEW_MASK_ARRAY_MATH_H

#include <iostream>
#include <casa/aips.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/namespace.h>

/// @brief Functions to return the median deviation from the median
/// for a masked array of data
/// @details The functions can take a variety of possible
/// parameters. The following is taken from the casacore information on
/// the median() function:
///
/// The median of "a" is a(n/2).
/// When a has an even number of elements and the switch takeEvenMean is set,
/// the median is 0.5*(a(n/2) + a((n+1)/2)).
/// According to Numerical Recipes (2nd edition) it makes little sense to take
/// the mean when the array is large enough (> 100 elements). Therefore
/// the default for takeEvenMean is False when the array has > 100 elements,
/// otherwise it is True.
/// <br>If "sorted"==True we assume the data is already sorted and we
/// compute the median directly. Otherwise the function GenSort::kthLargest
/// is used to find the median (kthLargest is about 6 times faster
/// than a full quicksort).
/// @name
/// @{
template<class T> inline T madfm(const MaskedArray<T> &a)
    { return median (a, False, (a.nelements() <= 100)); }
template<class T> inline T madfm(const MaskedArray<T> &a, Bool sorted)
    { return median (a, sorted, (a.nelements() <= 100)); }
template<class T> T madfm(const MaskedArray<T> &a, Bool sorted,
			   Bool takeEvenMean);
/// @}

/// @brief A class to find the median absolute deviation from the median of some masked data
/// @details This object takes a masked array of data and finds the median
/// absolute deviation from the median of it. This class is suitable to be
/// called by functions like slidingArrayMath. It can take flags to
/// indicate that the data is sorted, that the mean of the central two
/// points should be taken, or whether to do the calculations in the given
/// array.
template<typename T> class MaskedMadfmFunc {
    public:
        /// @brief Constructor
        explicit MaskedMadfmFunc(Bool sorted = False, Bool takeEvenMean = True)
                : itsSorted(sorted), itsTakeEvenMean(takeEvenMean) {}
        /// @brief Return the MADFM value
        Float operator()(const MaskedArray<Float>& arr) const
        { return madfm(arr, itsSorted, itsTakeEvenMean); }
    private:
        Bool     itsSorted;
        Bool     itsTakeEvenMean;
        Bool     itsInPlace;
};

#include <mathsutils/NewMaskArrMath.tcc>


#endif
