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

// own includes
#include "askap/IndexedCompare.h"
#include "askap/AskapError.h"
#include "utils/EigenDecompose.h"
#include "utils/SharedGSLTypes.h"

// GSL includes
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_eigen.h>

// std includes
#include <vector>
#include <algorithm>

namespace askap {

namespace utility {

/// @brief helper class acting as a random access iterator to a gsl vector
/// @details The gsl vector is held using reference semantics via the shared pointer
/// @ingroup utils
struct GSLVectorRAIterator {
   /// @brief value type
   typedef double value_type;
   /// @brief construct the object
   /// @param[in] vect gsl vector
   /// @param[in] elem element index pointed by this iterator
   explicit GSLVectorRAIterator(const utility::SharedGSLVector &vect, casa::uInt elem = 0) : itsIndex(elem), itsVector(vect) {}
   
   /// @brief advance iterator
   /// @details step how far to advance
   /// @return resulting iterator
   GSLVectorRAIterator operator+(const casa::uInt step) const { return GSLVectorRAIterator(itsVector, itsIndex + step);}

   /// @brief obtain data
   /// @return current element
   double operator*() { return gsl_vector_get(itsVector.get(),itsIndex);}
private:
   /// @brief current element   
   casa::uInt itsIndex;
   /// @brief gsl vector
   utility::SharedGSLVector itsVector;
};

} // namespace scimath

namespace scimath {

/// @brief eigen decomposition of a symmetric real matrix
/// @details A vector of eigenvalues and a matrix with eigen vectors are returned (and resized to a proper size)
/// @param[in] mtr input matrix (should be symmetric square matrix)
/// @param[out] eVal vector with eigen values (sorted from largest to smallest)
/// @param[out] eVect matrix with eigen vectors (in columns)
/// @ingroup utils
void symEigenDecompose(const casa::Matrix<double> &mtr, casa::Vector<double> &eVal, casa::Matrix<double> &eVect)
{
    const casa::uInt size = mtr.nrow();
    ASKAPCHECK(size == mtr.ncolumn(), "Expect a square matrix, you have "<<size<<" x "<<mtr.ncolumn()<<" matrix.");
    eVal.resize(size);
    eVect.resize(size,size);
         
    utility::SharedGSLMatrix A = utility::createGSLMatrix(size,size);
    utility::SharedGSLMatrix gslEVect = utility::createGSLMatrix(size,size);
    boost::shared_ptr<gsl_eigen_symmv_workspace> work = utility::createGSLObject(gsl_eigen_symmv_alloc(size));
    utility::SharedGSLVector gslEVal = utility::createGSLVector(size);

    for (casa::uInt row = 0; row<size; ++row) {
         for (casa::uInt col = 0; col<size; ++col) {
              gsl_matrix_set(A.get(), row, col, mtr(row,col));
         }
    }
         
    const int status = gsl_eigen_symmv(A.get(),gslEVal.get(),gslEVect.get(),work.get());
    
    if (status == GSL_SUCCESS) {
        std::vector<casa::uInt> indices(size);
        // initialise indices
        for (casa::uInt elem = 0; elem<size; ++elem) {
             indices[elem] = elem;
        }
        
        std::sort(indices.begin(),indices.end(),utility::indexedCompare<casa::uInt>(utility::GSLVectorRAIterator(gslEVal),
                  std::greater<double>()));
       
        for (casa::uInt elem = 0; elem<size; ++elem) {
             const casa::uInt index = indices[elem];
             ASKAPDEBUGASSERT(index<size);
             eVal[elem] = gsl_vector_get(gslEVal.get(),index);
             // extract the appropriate eigenvector
             for (casa::uInt i=0; i<size; ++i) {
                  eVect(i,elem) = gsl_matrix_get(gslEVect.get(),i,index);                             
             }         
        }
    }
                 
    ASKAPCHECK(status == GSL_SUCCESS, "Error solving eigenproblem in scimath::symmEigenDecompose, status="<<status);
}

} // namespace scimath

} // namespace askap


