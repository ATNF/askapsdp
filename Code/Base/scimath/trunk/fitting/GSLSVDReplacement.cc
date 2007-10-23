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

// own includes
#include <fitting/GSLSVDReplacement.h>
#include <conrad/ConradError.h>

// main SVD code, taken from another project
#include <fitting/SVDecompose.h>

// std includes
#include <vector>
#include <string>
#include <iostream>

namespace conrad {

namespace scimath {

/// @brief main method - do SVD (in a symmetric case)
/// @details The routine does decomposition: A=UWV^T
/// @param[in] A a matrix to decompose (gsl_matrix)
/// @param[out] V a matrix with eigenvectors
/// @param[out] S a vector with singular values
void SVDecomp(gsl_matrix *A, gsl_matrix *V, gsl_vector *S)
{
  // this is a test and probably temporary code to replace GSL's svd
  // routine with that stolen from another project (in SVDecompose.h)
  // this adapter method does additional copying (i.e. the goal is
  // quick implementation rather than a performance)
  CONRADDEBUGASSERT(A!=NULL);
  CONRADDEBUGASSERT(V!=NULL);
  CONRADDEBUGASSERT(S!=NULL);
 
  std::vector<double> MatrixABuffer;
  svd::Matrix2D<std::vector<double> > MatrixA(MatrixABuffer,A->size1,A->size2);
  
  std::vector<double> MatrixVBuffer;
  svd::Matrix2D<std::vector<double> > MatrixV(MatrixVBuffer);
  
  std::vector<double> VectorS;
  for (int row=0;row<MatrixA.nrow();++row) {
       for (int col=0;col<MatrixA.ncol();++col) {
            MatrixA(row,col)=gsl_matrix_get(A,row,col);
       }
  }
  try {
     computeSVD(MatrixA,VectorS,MatrixV);
  }
  catch(const std::string &str) {
      CONRADTHROW(ConradError, "SVD failed to converge: "<<str);
  }
  CONRADDEBUGASSERT(MatrixV.nrow() == V->size1);
  CONRADDEBUGASSERT(MatrixV.ncol() == V->size2);
  
  for (int row=0;row<MatrixV.nrow();++row) {
       for (int col=0;col<MatrixV.ncol();++col) {
            gsl_matrix_set(V,row,col,MatrixV(row,col));
       }
  }
  CONRADDEBUGASSERT(VectorS.size() == S->size);
  for (int item=0;item<VectorS.size();++item) {
       gsl_vector_set(S,item,VectorS[item]);
  }
  for (int row=0;row<MatrixA.nrow();++row) {
       for (int col=0;col<MatrixA.ncol();++col) {
            gsl_matrix_set(A,row,col,MatrixA(row,col));
       }
  }
}


} // namespace scimath

} // namespace conrad