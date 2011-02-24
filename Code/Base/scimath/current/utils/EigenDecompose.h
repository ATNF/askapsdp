/// @file 
/// @brief Eigen decomposition of casa matrices via gsl
/// @details It is handy to have eigen decomposition and related routines avaialble 
/// for casa matrices. This collection of methods wraps around GSL to provide this 
/// functionality.
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef EIGEN_DECOMPOSE_H
#define EIGEN_DECOMPOSE_H

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

namespace askap {

namespace scimath {

/// @brief eigen decomposition of a symmetric real matrix
/// @details A vector of eigenvalues and a matrix with eigen vectors are returned (and resized to a proper size)
/// @param[in] mtr input matrix (should be symmetric square matrix)
/// @param[out] eVal vector with eigen values (sorted from largest to smallest)
/// @param[out] eVect matrix with eigen vectors (in columns)
void symmEigenDecompose(const casa::Matrix<double> &mtr, casa::Vector<double> &eVal, casa::Matrix<double> &eVect);

} // namespace scimath

} // namespace askap


#endif // #ifndef EIGEN_DECOMPOSE_H

