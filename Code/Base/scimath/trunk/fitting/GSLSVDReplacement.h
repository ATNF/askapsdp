/// @file
/// 
/// @brief Singular value decomposition acting on GSL matrix/vector
/// @details This file contains a function, which acts as a replacement
/// of the GSL's gsl_linalg_SV_decomp by having the same interface.
/// It uses the SVD code from SVDecompose.h instead of the GSL. 
/// I hope that eventually this file will be dropped, as either the GSL
/// will be fixed or the code will be rewritten to completely avoid using GSL.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef GSLSVD_REPLACEMENT_H
#define GSLSVD_REPLACEMENT_H

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>


namespace conrad {

namespace scimath {

/// @brief main method - do SVD (in a symmetric case)
/// @details The routine does decomposition: A=UWV^T
/// @param[in] A a matrix to decompose (gsl_matrix)
/// @param[out] V a matrix with eigenvectors
/// @param[out] S a vector with singular values
void SVDecomp(gsl_matrix *A, gsl_matrix *V, gsl_vector *S);
        

} // namespace scimath

} // namespace conrad


#endif // #ifndef GSLSVD_REPLACEMENT_H