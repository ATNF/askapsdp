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

// own includes
#include <fitting/GSLSVDReplacement.h>
#include <askap/AskapError.h>
#include <askap/IndexedCompare.h>

// main SVD code, taken from another project
#include <fitting/SVDecompose.h>

// std includes
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>

namespace askap {

namespace utility {
/// @brief a helper object function to populate an array of indices
/// @details An instance of this class is initialized with an initial
/// number, which are returned in the first call. All subsequent calls
/// result in an incremented value. (result = initial + (number_of_call-1);)
template<typename T>
struct Counter {
   /// @brief type of the result
   typedef T result_type;
   /// @brief constructor
   /// @details initialize the counter with the initial value (default is 0)
   /// @param[in] val initial value
   Counter(const T &val = T(0)) : itsValue(val) {}
   
   /// @brief next value
   /// @details return current value and increment it
   T operator()() const { return itsValue++; }
private:
    /// current value
    mutable T itsValue;
};

} // namespace utility

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
  ASKAPDEBUGASSERT(A!=NULL);
  ASKAPDEBUGASSERT(V!=NULL);
  ASKAPDEBUGASSERT(S!=NULL);
 
  std::vector<double> MatrixABuffer;
  svd::Matrix2D<std::vector<double> > MatrixA(MatrixABuffer,A->size1,A->size2);
  
  std::vector<double> MatrixVBuffer;
  svd::Matrix2D<std::vector<double> > MatrixV(MatrixVBuffer);
  
  std::vector<double> VectorS;
  for (size_t row=0;row<MatrixA.nrow();++row) {
       for (size_t col=0;col<MatrixA.ncol();++col) {
            MatrixA(row,col)=gsl_matrix_get(A,row,col);
       }
  }
  try {
     computeSVD(MatrixA,VectorS,MatrixV);
  }
  catch(const std::string &str) {
      ASKAPTHROW(AskapError, "SVD failed to converge: "<<str);
  }
  ASKAPDEBUGASSERT(MatrixV.nrow() == V->size1);
  ASKAPDEBUGASSERT(MatrixV.ncol() == V->size2);
  // we need to sort singular values to get them in the descending order
  // with appropriate permutations of the A and V matrices
  std::vector<size_t> VectorSIndices(VectorS.size());
  std::generate(VectorSIndices.begin(), VectorSIndices.end(), 
                utility::Counter<size_t>());
  
  /*
  std::ofstream os("dbg.dat");
  os<<"   double Vals["<<VectorSIndices.size()<<"]={";
  for (size_t ii=0;ii<VectorS.size();++ii) {
       os<<std::setprecision(15)<<VectorS[ii];
       if (ii+1!=VectorS.size()) os<<",";
       if (ii%5==0 && ii) os<<std::endl<<"                    ";
  }
  os<<"};"<<std::endl;
  */
  
  std::sort(VectorSIndices.begin(), VectorSIndices.end(), 
        utility::indexedCompare<size_t>(VectorS.begin(),std::greater<double>()));
          
  ASKAPDEBUGASSERT(VectorS.size() == VectorSIndices.size());
  for (size_t row=0;row<MatrixV.nrow();++row) {
       for (size_t col=0;col<MatrixV.ncol();++col) {
            gsl_matrix_set(V,row,col,MatrixV(row,VectorSIndices[col]));
       }
  }
  ASKAPDEBUGASSERT(VectorS.size() == S->size);
  for (size_t item=0;item<VectorS.size();++item) {
       gsl_vector_set(S,item,VectorS[VectorSIndices[item]]);
  }
  for (size_t row=0;row<MatrixA.nrow();++row) {
       for (size_t col=0;col<MatrixA.ncol();++col) {
            gsl_matrix_set(A,row,col,MatrixA(row,VectorSIndices[col]));
       }
  }
}


} // namespace scimath

} // namespace askap

