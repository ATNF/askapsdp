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


#ifndef COMPLEXDIFFMATRIX_H
#define COMPLEXDIFFMATRIX_H

// casa includes
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

// std includes
#include <vector>
#include <string>
#include <map>

// own includes
#include <fitting/ComplexDiff.h>
#include <askap/AskapError.h>
#include <askap/MapKeyIterator.h>

namespace askap {

namespace scimath {

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
struct ComplexDiffMatrix {
   /// @brief constant iterator type used to access flattened storage
   typedef std::vector<ComplexDiff>::const_iterator const_iterator;
   
   /// @brief iterator corresponding to the origin of the sequence
   /// @return iterator corresponding to the origin of the sequence
   inline const_iterator begin() const { return itsElements.begin(); }
   
   /// @brief iterator corresponding to the end of the sequence
   /// @return iterator corresponding to the end of the sequence
   inline const_iterator end() const { return itsElements.end(); }
   
   /// @brief constructor of an empty matrix with the given dimensions
   /// @param[in] nrow number of rows
   /// @param[in] ncol number of columns
   explicit inline ComplexDiffMatrix(size_t nrow, size_t ncol = 1) : itsNRows(nrow),
               itsNColumns(ncol), itsElements(nrow*ncol), 
               itsParameterMapInvalid(true) {}
   
   /// @brief constructor of an initialized matrix with the given dimensions
   /// @param[in] nrow number of rows
   /// @param[in] ncol number of columns
   /// @param[in] val value
   inline ComplexDiffMatrix(size_t nrow, size_t ncol, const ComplexDiff &val) : 
         itsNRows(nrow), itsNColumns(ncol), itsElements(nrow*ncol, val),
         itsParameterMapInvalid(true) {}
   
   /// @brief constructor of an initialized vector with the given length
   /// @param[in] nrow number of rows
   /// @param[in] val value
   inline ComplexDiffMatrix(size_t nrow, const ComplexDiff &val) : 
         itsNRows(nrow), itsNColumns(1), itsElements(nrow, val),
         itsParameterMapInvalid(true) {}
         
   /// @brief constructor from casa::Matrix
   /// @param[in] matr input matrix
   inline ComplexDiffMatrix(const casa::Matrix<casa::Complex> &matr);    

   /// @brief constructor from casa::Vector
   /// @param[in] vec input matrix
   inline ComplexDiffMatrix(const casa::Vector<casa::Complex> &vec);    
   
   /// @brief access to given matrix element
   /// @param[in] row row
   /// @param[in] col column
   /// @return element
   inline const ComplexDiff& operator()(size_t row, size_t col) const;
   
   /// @brief read/write access to given matrix element
   /// @param[in] row row
   /// @param[in] col column
   /// @return element
   inline ComplexDiff& operator()(size_t row, size_t col);
   
   /// @brief access to given matrix element for a vector
   /// @param[in] index index of the element (row number)
   /// @return a const reference to the element
   inline const ComplexDiff& operator[](size_t index) const;
   
   /// @brief read-write access to a vector element
   /// @param[in] index index of the element (row number)
   /// @return a non-const reference to the element
   inline ComplexDiff& operator[](size_t index);
   
   /// @brief in-situ multiplication by a scalar
   /// @details 
   /// @param[in] scalar a ComplexDiff scalar
   inline void operator*=(const ComplexDiff &scalar);
   
   /// @brief in-situ multiplication by another ComplexDiffMatrix
   /// @details 
   /// @param[in] matr another ComplexDiffMatrix
   inline void operator*=(const ComplexDiffMatrix &matr);
   
   /// @brief matrix multiplication
   /// @details
   /// @param[in] in1 first matrix
   /// @param[in] in2 second matrix
   /// @return product of the first and the second matrices
   friend inline ComplexDiffMatrix operator*(const ComplexDiffMatrix &in1,
                const ComplexDiffMatrix &in2);
   
   /// @brief multiplication by a scalar
   /// @details This is an overloaded version of the multiplication operator,
   /// which is responsible for the case where the whole matrix is multiplied
   /// by a scalar from the right.
   /// @param[in] matr a ComplexDiffMatrix 
   /// @param[in] scalar a ComplexDiff scalar
   /// @return the product of the matrix and a scalar
   friend inline ComplexDiffMatrix operator*(const ComplexDiffMatrix &matr,
                 const ComplexDiff &scalar);
      
   /// @brief matrix addition
   /// @details
   /// @param[in] in1 first matrix
   /// @param[in] in2 second matrix
   /// @return a sum of the first and the second matrices
   friend inline ComplexDiffMatrix operator+(const ComplexDiffMatrix &in1,
                const ComplexDiffMatrix &in2);
  
   /// @brief obtain number of rows
   /// @return number of rows
   inline size_t nRow() const { return itsNRows;}
   
   /// @brief obtain number of columns
   /// @return number of columns
   inline size_t nColumn() const {return itsNColumns;}
   
   /// @brief return a total number of elements
   inline size_t nElements() const { return itsNRows*itsNColumns; }
   
   /// @brief type of the parameter iterator
   typedef utility::MapKeyIterator<std::map<std::string, bool>::const_iterator > 
           parameter_iterator;
   
   /// @brief iterator for the start of the parameter sequence
   /// @details This method serves as STL's begin() for a sequence of
   /// parameters known for all elements of this matrix.
   inline parameter_iterator paramBegin() const;
   
   /// @brief iterator for the end of the parameter sequence
   /// @details This method serves as STL's end() for a sequence of
   /// parameters known for all elements of this matrix.
   inline parameter_iterator paramEnd() const;
   
   /// @brief checks whether a given parameter is conceptually real
   /// @details Some parameters are conceptually real. Underlying 
   /// ComplexDiff classes don't track derivatives by imaginary part
   /// for these parameters. This method allows to check the type of
   /// a given parameter.
   /// @param[in] param parameter name
   /// @return true if the given parameter is always real
   inline bool isReal(const std::string &param) const { return itsParameters[param];}
   
protected:
    
   /// @brief build the list of all known parameters
   /// @details This method makes the cache up to date. It iterates through
   /// all elements of this matrix and builds a set of parameters they know
   /// about. The flag itsParameterSetInvalid is reset to false at the end.
   /// @note This method is conseptually constant as it works with the cache
   /// only.
   void buildParameterMap() const;
       
private:
   /// @brief number of rows (channels in the calibration framework)
   size_t itsNRows;
   /// @brief number of columns (polarisations in the calibration framework)
   size_t itsNColumns; 
   /// @brief flattened storage for the matrix elements
   std::vector<ComplexDiff> itsElements;
   
   /// @brief a list of all parameters known to the elements of this matrix
   /// @details The value of the map is true if corresponding parameter is
   /// conceptually real and false otherwise.
   mutable std::map<std::string, bool> itsParameters;
   
   /// @brief a flag showing that itsParameterMap needs to be updated
   mutable bool itsParameterMapInvalid;
};

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
inline ComplexDiffMatrix::ComplexDiffMatrix(const casa::Matrix<casa::Complex> &matr) :
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
inline ComplexDiffMatrix::ComplexDiffMatrix(const casa::Vector<casa::Complex> &vec) :
     itsNRows(vec.nelements()), itsNColumns(1), itsElements(vec.nelements()),
     itsParameterMapInvalid(true)    
{
   std::vector<ComplexDiff>::iterator it = itsElements.begin();
   for (casa::uInt row=0; row<itsNRows; ++row,++it) {
        *it = vec[row];
   }
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

#endif // #ifndef COMPLEXDIFFMATRIX_H
