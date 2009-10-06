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

#ifndef NEW_ARRAY_PART_MATH_H
#define NEW_ARRAY_PART_MATH_H

#include <iostream>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/namespace.h>
#include <analysisutilities/NewArrayMath.h>


template<typename T> class MadfmFunc {
public:
  explicit MadfmFunc (Bool sorted=False, Bool takeEvenMean=True,
                       Bool inPlace = False)
    : itsSorted(sorted), itsTakeEvenMean(takeEvenMean), itsInPlace(inPlace) {}
  Float operator() (const Array<Float>& arr) const
    { return madfm(arr, itsTmp, itsSorted, itsTakeEvenMean, itsInPlace); }
private:
  Bool     itsSorted;
  Bool     itsTakeEvenMean;
  Bool     itsInPlace;
  mutable Block<Float> itsTmp;
};

template<typename T> class SihrFunc {
public:
  explicit SihrFunc (Bool sorted=False, Bool inPlace = False)
    : itsSorted(sorted), itsInPlace(inPlace) {}
  Float operator() (const Array<Float>& arr) const
    { return sihr(arr, itsTmp, itsSorted, itsInPlace); }
private:
  Bool     itsSorted;
  Bool     itsInPlace;
  mutable Block<Float> itsTmp;
};

#include <analysisutilities/NewArrayPartMath.tcc>

#endif
