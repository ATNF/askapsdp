/// @file
/// 
/// @brief Singular value decomposition acting on GSL matrix/vector
/// @details This file contains a function, which acts as a replacement
/// of the GSL's gsl_linalg_SV_decomp by having the same interface.
/// It uses the SVD code from SVDecompose.h instead of the GSL. 
/// I hope that eventually this file will be dropped, as either the GSL
/// will be fixed or the code will be rewritten to completely avoid using GSL.
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

#ifndef GSLSVD_REPLACEMENT_H
#define GSLSVD_REPLACEMENT_H

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>


namespace askap {

namespace scimath {

/// @brief main method - do SVD (in a symmetric case)
/// @param[in] A a matrix to decompose (gsl_matrix)
/// @param[out] V a matrix with eigenvectors
/// @param[out] S a vector with singular values
/// @ingroup fitting
void SVDecomp(gsl_matrix *A, gsl_matrix *V, gsl_vector *S);
        

} // namespace scimath

} // namespace askap


#endif // #ifndef GSLSVD_REPLACEMENT_H

