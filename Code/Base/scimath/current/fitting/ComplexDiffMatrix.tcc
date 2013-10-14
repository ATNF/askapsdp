/// @file
/// 
/// @brief A matrix of ComplexDiff classes
/// @details The calibration code constructs normal equations for each
/// row of the data accessor, i.e. a matrix with dimensions nchan x npol.
/// When a design matrix is constructed, all elements of this matrix are
/// treated independently. However, it is better to retain a basic matrix
/// algebra to ensure the code is clear. This class also treats well a
/// possible degenerate dimension (polarisation). Theoretically, 
/// casa::Matrix<ComplexDiff> could be used instead of this class. However,
/// having a separate class allows, in principle, to handle maps of the 
/// parameters at the matrix level and don't duplicate the map search 
/// unnecessarily. Such functionality is in the future plans, but it is
/// hidden behind the interface of this class.
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


#ifndef COMPLEXDIFFMATRIX_TCC
#define COMPLEXDIFFMATRIX_TCC

// own includes
#include <askap/AskapError.h>
#include <askap/MapKeyIterator.h>

namespace askap {

namespace scimath {

/// @brief multiplication by a scalar
/// @details This is an overloaded version of the multiplication operator,
/// which is responsible for the case where the whole matrix is multiplied
/// by a scalar from the left. 
/// @param[in] scalar a ComplexDiff scalar
/// @param[in] matr a ComplexDiffMatrix 
/// @return the product of the matrix and a scalar
inline  ComplexDiffMatrix operator*(const ComplexDiff &scalar, 
                 const ComplexDiffMatrix &matr) { return matr*scalar;}	 


/// @brief access to given matrix element
/// @param[in] row row
/// @param[in] col column
/// @return element
inline const ComplexDiff& ComplexDiffMatrix::operator()(size_t row, size_t col) const
{
   ASKAPDEBUGASSERT(row<itsNRows && col<itsNColumns);
   return itsElements[itsNRows*col+row];
}

/// @brief access to given matrix element
/// @param[in] row row
/// @param[in] col column
/// @return element
inline ComplexDiff& ComplexDiffMatrix::operator()(size_t row, size_t col)
{
   ASKAPDEBUGASSERT(row<itsNRows && col<itsNColumns);
   itsParameterMapInvalid = true;
   return itsElements[itsNRows*col+row];
}

/// @brief access to given matrix element for a vector
/// @param[in] index index of the element (row number)
/// @return a const reference to the element
inline const ComplexDiff& ComplexDiffMatrix::operator[](size_t index) const
{
   ASKAPDEBUGASSERT(itsNColumns == 1);
   return operator()(index,0);
}
   
/// @brief read-write access to a vector element
/// @param[in] index index of the element (row number)
/// @return a non-const reference to the element
inline ComplexDiff& ComplexDiffMatrix::operator[](size_t index)
{
   ASKAPDEBUGASSERT(itsNColumns == 1);
   return operator()(index,0);
}


/// @brief constructor from casa::Matrix
/// @param[in] matr input matrix
template<typename T>
inline ComplexDiffMatrix::ComplexDiffMatrix(const casa::Matrix<T> &matr) :
     itsNRows(matr.nrow()), itsNColumns(matr.ncolumn()), 
     itsElements(matr.nrow()*matr.ncolumn()), itsParameterMapInvalid(true)    
{
   std::vector<ComplexDiff>::iterator it = itsElements.begin();
   for (casa::uInt col=0; col<itsNColumns; ++col) {
        for (casa::uInt row=0; row<itsNRows; ++row,++it) {
             *it = matr(row,col);
        }
   }
}

/// @brief constructor from casa::Vector
/// @param[in] vec input vector
template<typename T>
inline ComplexDiffMatrix::ComplexDiffMatrix(const casa::Vector<T> &vec) :
     itsNRows(vec.nelements()), itsNColumns(1), itsElements(vec.nelements()),
     itsParameterMapInvalid(true)    
{
   std::vector<ComplexDiff>::iterator it = itsElements.begin();
   for (casa::uInt row=0; row<itsNRows; ++row,++it) {
        *it = vec[row];
   }
}

/// @brief matrix addition allowing block-matrices
/// @details It is often convenient to stitch matrices together into a 
/// block matrix (e.g. frequency-dependent Mueller matrix). However, at the
/// same time it is desireable to mix normal matrices (i.e. frequency-independent
/// effects) with block matrices. This method deals with addition assuming 
/// a certain block representation (essentially a sparse diagonal matrix).
/// @param[in] in1 first matrix
/// @param[in] in2 second matrix
/// @return sum of the first and the second block-matrices
inline ComplexDiffMatrix blockAdd(const ComplexDiffMatrix &in1,
                const ComplexDiffMatrix &in2) 
{
  if ((in1.nColumn() == in2.nColumn()) && (in1.nRow() == in2.nRow())) {
      // simple case of either ordinary matrices or two matching block matrices
      // either way it is a simple element by element addition
      return in1 + in2;
  }
  // there are two options here: block + ordinary and ordinary + block
  ASKAPDEBUGASSERT(in1.nRow() == in2.nRow());
  // the result doesn't depend on the order, so just get the smallest number of columns and treat the corresponding
  // matrix as the ordinary matrix
  const ComplexDiffMatrix &ordinary = in1.nColumn()<in2.nColumn() ? in1 : in2;
  const ComplexDiffMatrix &block = in1.nColumn()<in2.nColumn() ? in2 : in1;
  ASKAPCHECK(block.nColumn() % ordinary.nColumn() == 0, 
     "Block matrix is supposed to have the number of columns which is an integral multiple of number of columns for the other matrix ("<<
     ordinary.nColumn()<<"), you have "<<block.nColumn());

  ComplexDiffMatrix result(block.nRow(), block.nColumn());
  size_t curCol = 0, curRow = 0;
  for (std::vector<ComplexDiff>::iterator it = result.itsElements.begin();
       it != result.itsElements.end(); ++it,++curRow) {
       *it = ComplexDiff(casa::Complex(0.,0.));
       if (curRow >= result.nRow()) {
           curRow = 0;
           ++curCol;
       }
       ASKAPDEBUGASSERT(curCol < block.nColumn());
       *it = block(curRow,curCol) + ordinary(curRow,curCol % ordinary.nColumn());
  }
  return result;      
}

/// @brief matrix multiplication allowing block-matrices
/// @details We really work with multi-dimensional data rather than vectors.
/// Therefore, it is often convenient to stitch matrices together into a block
/// matrix to bring another dimension in (one example is frequency-dependent
/// Mueller matrices). Blocks are considered independent. So for a multiplication
/// of a n x m matrix and a p x q matrix (n%p == 0 and q%(n/p) == 0) each n x p block of
/// the first matrix is multiplied by the same or corresponding block of the second.
/// if the first matrix is n x n, the case reversed to normal multiplication. 
/// @param[in] in1 first matrix
/// @param[in] in2 second matrix
/// @return product of the first and the second block-matrices
inline ComplexDiffMatrix blockMultiply(const ComplexDiffMatrix &in1,
                const ComplexDiffMatrix &in2)
{
  if (in1.nColumn() == in2.nRow()) {
      // this is the case of normal matrix multiplication
      return in1 * in2;
  }
  // there are two options here
  // 1. block matrix x ordinary matrix
  // 2. block matrix x block matrix with matching number of blocks
  // ordinary matrix x block matrix is taken care of by the standard case
  ASKAPDEBUGASSERT(in1.nRow() > 0);
  ASKAPDEBUGASSERT(in2.nRow() > 0);
  ASKAPDEBUGASSERT(in1.nColumn() % in2.nRow() == 0);
  const casa::uInt nBlocks = in1.nColumn() / in2.nRow();
  if (in2.nColumn() == in1.nRow()) {
      // 1. block matrix x ordinary matrix -> blocks of square matrices
      ComplexDiffMatrix result(in1.nRow(), in2.nColumn() * nBlocks); // output is a block-matrix
      size_t curCol = 0, curRow = 0;
      for (std::vector<ComplexDiff>::iterator it = result.itsElements.begin();
           it != result.itsElements.end(); ++it,++curRow) {
           *it = ComplexDiff(casa::Complex(0.,0.));
           if (curRow >= result.nRow()) {
               curRow = 0;
               ++curCol;
           }
           const size_t block = curCol  - curCol % in2.nRow();
           for (size_t index=0; index<in2.nRow(); ++index) {
                *it += in1(curRow, block + index)*in2(index,curCol % in2.nColumn());
           }
      }
      return result;      
  } 
  // 2. block matrix x block matrix with matching number of blocks
  ASKAPDEBUGASSERT(in2.nColumn() % nBlocks == 0);
  
  ComplexDiffMatrix result(in1.nRow(), in2.nColumn()); // output is a block-matrix
  size_t curCol = 0, curRow = 0;
  for (std::vector<ComplexDiff>::iterator it = result.itsElements.begin();
       it != result.itsElements.end(); ++it,++curRow) {
       *it = ComplexDiff(casa::Complex(0.,0.));
       if (curRow >= result.nRow()) {
           curRow = 0;
           ++curCol;
       }
       const size_t block = curCol  - curCol % in2.nRow();
       for (size_t index=0; index<in2.nRow(); ++index) {
            *it += in1(curRow, block + index)*in2(index,curCol);
       }
  }
  return result;      
}                


/// @brief matrix multiplication
/// @details
/// @param[in] in1 first matrix
/// @param[in] in2 second matrix
/// @return product of the first and the second matrices
inline ComplexDiffMatrix operator*(const ComplexDiffMatrix &in1,
                const ComplexDiffMatrix &in2)
{
   ASKAPDEBUGASSERT(in1.nColumn() == in2.nRow());
   ComplexDiffMatrix result(in1.nRow(), in2.nColumn());
   size_t curCol = 0, curRow = 0;
   for (std::vector<ComplexDiff>::iterator it = result.itsElements.begin();
        it != result.itsElements.end(); ++it,++curRow) {
        *it = ComplexDiff(casa::Complex(0.,0.));
        if (curRow >= result.nRow()) {
            curRow = 0;
            ++curCol;
        }
        for (size_t index=0; index<in1.nColumn(); ++index) {
             *it += in1(curRow,index)*in2(index,curCol);
        }
   }
   return result;
}

/// @brief multiplication by a scalar
/// @details This is an overloaded version of the multiplication operator,
/// which is responsible for the case where the whole matrix is multiplied
/// by a scalar from the right.
/// @param[in] matr a ComplexDiffMatrix 
/// @param[in] scalar a ComplexDiff scalar
/// @return the product of the matrix and a scalar
inline ComplexDiffMatrix operator*(const ComplexDiffMatrix &matr,
                 const ComplexDiff &scalar)
{
  ComplexDiffMatrix result(matr);
  for (std::vector<ComplexDiff>::iterator it = result.itsElements.begin();
             it != result.itsElements.end(); ++it) {
       *it *= scalar;
  }
  return result;  
}

/// @brief in-situ multiplication by a scalar
/// @details 
/// @param[in] scalar a ComplexDiff scalar
inline void ComplexDiffMatrix::operator*=(const ComplexDiff &scalar)
{
  for (std::vector<ComplexDiff>::iterator it = itsElements.begin(); 
                       it != itsElements.end(); ++it) {
       *it *= scalar;
  }  
}
   
/// @brief in-situ multiplication by another ComplexDiffMatrix
/// @details 
/// @param[in] matr another ComplexDiffMatrix
inline void ComplexDiffMatrix::operator*=(const ComplexDiffMatrix &matr)
{
   operator=(*this * matr);
}

/// @brief matrix addition
/// @details
/// @param[in] in1 first matrix
/// @param[in] in2 second matrix
/// @return a sum of the first and the second matrices
inline ComplexDiffMatrix operator+(const ComplexDiffMatrix &in1,
                const ComplexDiffMatrix &in2)
{
  ASKAPDEBUGASSERT(in1.nColumn() == in2.nColumn());
  ASKAPDEBUGASSERT(in1.nRow() == in2.nRow());
  ComplexDiffMatrix result(in1);
  ComplexDiffMatrix::const_iterator ci = in2.begin();
  for (std::vector<ComplexDiff>::iterator it = result.itsElements.begin();
             it != result.itsElements.end(); ++it,++ci) {
       *it += *ci;
  }
  return result;
}

/// @brief iterator for the start of the parameter sequence
/// @details This method serves as STL's begin() for a sequence of
/// parameters known for all elements of this matrix.
inline ComplexDiffMatrix::parameter_iterator ComplexDiffMatrix::paramBegin() const
{
  if (itsParameterMapInvalid) {
      buildParameterMap();
  }
  return utility::mapKeyBegin(itsParameters);
}
   
/// @brief iterator for the end of the parameter sequence
/// @details This method serves as STL's end() for a sequence of
/// parameters known for all elements of this matrix.
inline ComplexDiffMatrix::parameter_iterator ComplexDiffMatrix::paramEnd() const
{
  if (itsParameterMapInvalid) {
      buildParameterMap();
  }
  return utility::mapKeyEnd(itsParameters);
}



} // namepace scimath

} // namespace askap

#endif // #ifndef COMPLEXDIFFMATRIX_TCC
