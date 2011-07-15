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
  CPPUNIT_TEST_SUITE_END();
protected:
  /// @brief helper method to check that two complex numbers match
  /// @param[in] expected expected value
  /// @param[in] actual value to test
  void compareComplex(const casa::Complex &expected, const casa::Complex &actual) {
       const double tolerance = 1e-7;
       CPPUNIT_ASSERT_DOUBLES_EQUAL(double(casa::real(expected)),double(casa::real(actual)),tolerance);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(double(casa::imag(expected)),double(casa::imag(actual)),tolerance);
  }
public:
  void testConstruct() {
     PolXProducts pxp(4,casa::IPosition(2,3,5),true);
     // check that internal buffers are constructed with correct dimensions
     // (i.e. no exceptions have been thrown)
     for (casa::uInt x=0; x<3; ++x) {
          for (casa::uInt y=0; y<5; ++y) {
               for (casa::uInt p1=0; p1<4; ++p1) {
                    for (casa::uInt p2=0; p2<4; ++p2) {
                         compareComplex(0., pxp.getModelProduct(x,y,p1,p2));
                         compareComplex(0., pxp.getModelMeasProduct(x,y,p1,p2));                         
                    }
               }
          }
     }
  }
};

} // namespace scimath

} // namespace askap

#endif // #ifndef POL_X_PRODUCTS_TEST


