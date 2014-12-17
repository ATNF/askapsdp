/// @file
///
/// @brief test of gsl pointer wrappers
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

// unit test include
#include <cppunit/extensions/HelperMacros.h>

// own includes
#include "utils/SharedGSLTypes.h"

// gsl includes
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>


namespace askap {

namespace utility {

/// @brief helper specialisation to allow testing of the destructor call
/// @details It is not used anywhere outside this test suit 
template<> struct CustomGSLDeleter<bool> {
    /// @brief this method just sets the passed object to true
    /// @param[in] obj pointer to a boolean variable
    void operator()(bool *obj) const { *obj = true; }
};


class SharedGSLTypesTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(SharedGSLTypesTest);
   CPPUNIT_TEST(testVector);
   CPPUNIT_TEST(testMatrix);
   CPPUNIT_TEST_EXCEPTION(testNullPointer,AskapError);     
   CPPUNIT_TEST(testDestruction); 
   CPPUNIT_TEST_SUITE_END();
public:
   void testVector() {
      const size_t nElements = 10;
      SharedGSLVector vec = createGSLVector(nElements);
      for (size_t el = 0; el < nElements; ++el) {
           gsl_vector_set(vec.get(), el, double(el));
      }
      // check the content (can't really check that the destructor is
      // called)
      for (size_t el = 0; el < nElements; ++el) {
          CPPUNIT_ASSERT_DOUBLES_EQUAL(double(el), gsl_vector_get(vec.get(), el), 1e-6);
      }
      // destructor should be called on leaving this method      
   }

   void testMatrix() {
      const size_t nRow = 10;
      const size_t nCol = 12;
      SharedGSLMatrix matr = createGSLMatrix(nRow, nCol);
      for (size_t row = 0; row < nRow; ++row) {
           for (size_t col = 0; col < nCol; ++col) {
                gsl_matrix_set(matr.get(), row, col, double(row*col));
           }
      }
      
      // check the content (can't really check that the destructor is
      // called)
      for (size_t row = 0; row < nRow; ++row) {
           for (size_t col = 0; col < nCol; ++col) {
                CPPUNIT_ASSERT_DOUBLES_EQUAL(double(row*col), gsl_matrix_get(matr.get(), row,col), 1e-6);
           }
      }
      // destructor should be called on leaving this method      
   }
   
   void testNullPointer() {
      gsl_vector *nullVec = NULL;
      // the following should throw an exception
      createGSLObject(nullVec);
   }
   
   void testDestruction() {
      bool destructorCalledBuf = false;
      {
        const boost::shared_ptr<bool> destructorCalledPtr = createGSLObject(&destructorCalledBuf);
        CPPUNIT_ASSERT(destructorCalledPtr);
        CPPUNIT_ASSERT_EQUAL(false, *destructorCalledPtr);
        CPPUNIT_ASSERT_EQUAL(false, destructorCalledBuf);        
      }
      CPPUNIT_ASSERT_EQUAL(true, destructorCalledBuf);      
   }
};

} // namespace utility

} // namespace askap

