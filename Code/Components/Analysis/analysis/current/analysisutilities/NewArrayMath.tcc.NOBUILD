/// @file
///
/// Additional functions to go with those in casa/Arrays/ArrayMath.tcc
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
#include <casa/namespace.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <analysisutilities/NewArrayMath.h>

template<class T> T madfm(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool takeEvenMean, Bool inPlace)
{

    T med = median(a, tmp, sorted, takeEvenMean, inPlace);

    Array<T> absdiff = abs(a - med);
    return median(absdiff, tmp, sorted, takeEvenMean, inPlace);

}
//template<Float> madfm(const Array<Float> &a, Block<Float> &tmp, Bool sorted, Bool takeEvenMean, Bool inPlace);

template<class T> T sihr(const Array<T> &a, Block<T> &tmp, Bool sorted, Bool inPlace)
{

    T hex1 = fractile(a, tmp, 1. / 6., sorted, inPlace);
    T hex5 = fractile(a, tmp, 5. / 6., sorted, inPlace);

    return T(0.5*(hex5 - hex1));
}
