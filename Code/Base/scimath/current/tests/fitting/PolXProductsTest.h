/// @file
/// 
/// @brief Tests of PolXProducts, a helper class for pre-summing calibration
/// @details See PolXProducts for description of what this class  
/// is supposed to do. This file contains appropriate unit tests.
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

#ifndef POL_X_PRODUCTS_TEST
#define POL_X_PRODUCTS_TEST

#include <cppunit/extensions/HelperMacros.h>
#include <fitting/PolXProducts.h>
#include <askap/AskapError.h>

namespace askap {

namespace scimath {

class PolXProductsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PolXProductsTest);
  CPPUNIT_TEST(testConstruct);
  CPPUNIT_TEST(testConstructVector);
  CPPUNIT_TEST(testSlice);
  CPPUNIT_TEST(testResize);
  CPPUNIT_TEST(testPolIndices);
#ifdef ASKAP_DEBUG  
  // dimension mismatch is detected in the debug mode only
  CPPUNIT_TEST_EXCEPTION(testDimensionMismatch, askap::AssertError);
#endif // #ifdef ASKAP_DEBUG
  CPPUNIT_TEST_SUITE_END();  
protected:
  /// @brief helper method to check that two complex numbers match
  /// @param[in] expected expected value
  /// @param[in] actual value to test
  static void compareComplex(const casa::Complex &expected, const casa::Complex &actual) {
       const double tolerance = 1e-7;
       CPPUNIT_ASSERT_DOUBLES_EQUAL(double(casa::real(expected)),double(casa::real(actual)),tolerance);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(double(casa::imag(expected)),double(casa::imag(actual)),tolerance);
  }
  
  /// @brief helper method to check buffers 
  /// @param[in] pxp cross-products buffer to test
  /// @param[in] nX number of pixels for the first dimension
  /// @param[in] nY number of pixels for the second dimension
  static void checkAllElementsAreZero(const PolXProducts &pxp, casa::uInt nX, casa::uInt nY) {
     for (casa::uInt x=0; x<nX; ++x) {
          for (casa::uInt y=0; y<nY; ++y) {
               for (casa::uInt p1=0; p1<pxp.nPol(); ++p1) {
                    for (casa::uInt p2=0; p2<pxp.nPol(); ++p2) {
                         compareComplex(0., pxp.getModelProduct(x,y,p1,p2));
                         compareComplex(0., pxp.getModelMeasProduct(x,y,p1,p2));                         
                    }
               }
          }
     }     
  }
  
public:
  void testConstruct() {
     PolXProducts pxp(4,casa::IPosition(2,3,5),true);
     CPPUNIT_ASSERT_EQUAL(4u,pxp.nPol());
     // check that internal buffers are constructed with correct dimensions
     // (i.e. no exceptions have been thrown)
     checkAllElementsAreZero(pxp,3,5);
  }
  
  void testConstructVector() {
     PolXProducts pxp(2,casa::IPosition(),true);
     CPPUNIT_ASSERT_EQUAL(2u,pxp.nPol());
     // check that internal buffers are constructed with correct dimensions
     // (i.e. no exceptions have been thrown)
     for (casa::uInt p1=0; p1<2; ++p1) {
          for (casa::uInt p2=0; p2<2; ++p2) {
               compareComplex(0., pxp.getModelProduct(p1,p2));
               compareComplex(0., pxp.getModelMeasProduct(p1,p2));                         
          }
     }
  }
  
  void testSlice() {
     PolXProducts pxp(4,casa::IPosition(2,3,5),true);
     CPPUNIT_ASSERT_EQUAL(4u,pxp.nPol());
     // fill the buffers with different values
     for (casa::uInt x=0; x<3; ++x) {
          for (casa::uInt y=0; y<5; ++y) {
               for (casa::uInt p1=0; p1<4; ++p1) {
                    for (casa::uInt p2=0; p2<=p1; ++p2) {
                         // unique value for every product
                         const float tagValue = 10.*x+100.*y+float(p1)+0.1*p2;
                         const casa::Complex cTag(tagValue,-tagValue);
                         pxp.add(x,y,p1,p2,cTag,-cTag);
                    }
               }
          }
     }
     // now check all slices
     for (casa::uInt x=0; x<3; ++x) {
          for (casa::uInt y=0; y<5; ++y) {
               const PolXProducts slice = pxp.slice(casa::IPosition(2,x,y));
               for (casa::uInt p1=0; p1<4; ++p1) {
                    for (casa::uInt p2=0; p2<=p1; ++p2) {
                         const float tagValue = 10.*x+100.*y+float(p1)+0.1*p2;
                         const casa::Complex cTag(tagValue,-tagValue);
                         compareComplex(cTag, slice.getModelProduct(p1,p2));
                         compareComplex(-cTag, slice.getModelMeasProduct(p1,p2));
                         if (p1 != p2) {
                             // test conjugation
                             compareComplex(cTag, conj(slice.getModelProduct(p2,p1)));
                             compareComplex(-cTag, conj(slice.getModelMeasProduct(p2,p1)));                             
                         }
                    }
               }
          }
     }     
  }
  void testResize() {
     PolXProducts pxp(2,casa::IPosition(),true);
     CPPUNIT_ASSERT_EQUAL(2u,pxp.nPol());
     pxp.resize(casa::IPosition(2,3,5), true);
     CPPUNIT_ASSERT_EQUAL(2u,pxp.nPol());
     // check that internal buffers are constructed with correct dimensions
     // (i.e. no exceptions have been thrown)
     checkAllElementsAreZero(pxp,3,5);
     // resize back
     pxp.resize(casa::IPosition(),false);
     pxp.reset();
     CPPUNIT_ASSERT_EQUAL(2u,pxp.nPol());
     // resize with a change of polarisation vector dimensions
     pxp.resize(4,casa::IPosition(2,3,5), true);
     CPPUNIT_ASSERT_EQUAL(4u,pxp.nPol());
     // check that internal buffers are constructed with correct dimensions
     // (i.e. no exceptions have been thrown)
     checkAllElementsAreZero(pxp,3,5);     
  }
  
  void testPolIndices() {
     // polToIndex and polFromIndex are protected, so we need a helper
     // class to test these methods
     struct TestHelper : public PolXProducts {
        TestHelper() : PolXProducts(4u) {}
        using PolXProducts::polToIndex;
        using PolXProducts::indexToPol;        
     };
     
     // instantiate test helper instead of the PolXProducts
     TestHelper pxp;
     CPPUNIT_ASSERT_EQUAL(4u,pxp.nPol());
     
     for (casa::uInt p1=0; p1<pxp.nPol(); ++p1) {
          for (casa::uInt p2=0; p2<=p1; ++p2) {
               const casa::uInt index = pxp.polToIndex(p1,p2);
               CPPUNIT_ASSERT(index < pxp.nPol()*(pxp.nPol()+1)/2);
               const std::pair<casa::uInt,casa::uInt> pols = pxp.indexToPol(index);
               CPPUNIT_ASSERT_EQUAL(p1, pols.first);
               CPPUNIT_ASSERT_EQUAL(p2, pols.second);               
          }
     }
  }
  
  // this is checked in the debug mode only!
  void testDimensionMismatch() {
     PolXProducts pxp(4,casa::IPosition(2,3,5),true);
     CPPUNIT_ASSERT_EQUAL(4u,pxp.nPol());
     // the following will throw an exception
     pxp.getModelProduct(0,1);     
  }
};

} // namespace scimath

} // namespace askap

#endif // #ifndef POL_X_PRODUCTS_TEST


