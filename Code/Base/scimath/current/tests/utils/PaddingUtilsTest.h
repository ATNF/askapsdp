/// @file
/// 
/// @brief test of the helper cache template representing a map of a fixed size 
/// @details Cache of some object can be based on a maps of shared pointers. Sometimes,
/// we need to limit the number of elements in the cache to stop map from growing infinitely.
/// This tested template provides such a cache class.  
///
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

#ifndef PADDING_UTILS_TEST_H
#define PADDING_UTILS_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <utils/PaddingUtils.h>
#include <casa/Arrays/Matrix.h>


namespace askap {

namespace scimath {

class PaddingUtilsTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(PaddingUtilsTest);
   CPPUNIT_TEST(testPaddedShape);
   CPPUNIT_TEST(testExtract);
   CPPUNIT_TEST(testNonIntegralPadding);
   CPPUNIT_TEST(testClip);
   CPPUNIT_TEST_SUITE_END();
public:
   void testPaddedShape() {
      const casa::IPosition shape(3,10,5,2);
      const casa::IPosition padded = PaddingUtils::paddedShape(shape,2);
      CPPUNIT_ASSERT(padded.nelements() == shape.nelements());
      CPPUNIT_ASSERT(padded[0] == 20);
      CPPUNIT_ASSERT(padded[1] == 10);
      CPPUNIT_ASSERT(padded[2] == 2);      
   }  
   
   void testExtract() {
      const casa::IPosition shape(2,3,2);
      casa::Matrix<bool> paddedArray(PaddingUtils::paddedShape(shape,2),false);
      PaddingUtils::extract(paddedArray,2).set(true);
      CPPUNIT_ASSERT(paddedArray.nrow() == 6);
      CPPUNIT_ASSERT(paddedArray.ncolumn() == 4);
      for (uint row=0; row<paddedArray.nrow(); ++row) {
           for (uint column=0; column<paddedArray.ncolumn(); ++column) {
                CPPUNIT_ASSERT(paddedArray(row,column) == ((row>=1) && (row<=3) && (column>=1) && (column<=2))); 
           }
      }     
   }
   
   void testNonIntegralPadding() {
      doNonIntegralPaddingTest(casa::IPosition(2,31,19),2.2);
      doNonIntegralPaddingTest(casa::IPosition(2,1,7),2.5234);
      doNonIntegralPaddingTest(casa::IPosition(2,32,64),casa::C::pi);
      doNonIntegralPaddingTest(casa::IPosition(2,32,63),sqrt(2.));
   }
   
   void testClip() {
      casa::Matrix<float> image(6,3,1.);
      PaddingUtils::clip(image,casa::IPosition(2,3,1));
      for (uint row=0; row<image.nrow(); ++row) {
           for (uint column=0; column<image.ncolumn();++column) {
                if ((column == 1) && (row>=1) && (row<=3)) {
                    CPPUNIT_ASSERT(fabs(image(row,column)-1.)<1e-6);
                } else {
                    CPPUNIT_ASSERT(fabs(image(row,column))<1e-6);
                }
           }
      }
      
      // larger scale test similar to practical use
      casa::Cube<float> cube(1024,512,2,1.);
      const casa::IPosition  innerShape(2,512,256);
      PaddingUtils::clip(cube,innerShape);      
      PaddingUtils::centeredSubArray(cube,innerShape.concatenate(casa::IPosition(1,2))).set(2.);
      for (uint plane=0; plane<cube.nplane(); ++plane) {
           for (uint row=0; row<cube.nrow(); ++row) {
                for (uint column=0; column<cube.ncolumn(); ++column) {
                     if (fabs(cube(row,column,plane)) > 1e-6) {
                         CPPUNIT_ASSERT(fabs(cube(row,column,plane)-2.)<1e-6);
                     }
                }
           }
      }
   }

protected:   
   /// @param[in] shape unpadded shape
   /// @param[in] factor padding factor
   void doNonIntegralPaddingTest(const casa::IPosition &shape, float factor) {
     CPPUNIT_ASSERT(shape.nelements() == 2);
     casa::Matrix<bool> paddedArray(PaddingUtils::paddedShape(shape,factor),false);
     casa::Matrix<bool> subArray = PaddingUtils::extract(paddedArray,factor);
     subArray.set(true);
     const casa::uInt expectedX = casa::uInt(std::floor(factor*shape[0]));
     const casa::uInt expectedY = casa::uInt(std::floor(factor*shape[1]));     
     CPPUNIT_ASSERT(paddedArray.nrow() == expectedX);
     CPPUNIT_ASSERT(paddedArray.ncolumn() == expectedY);
     CPPUNIT_ASSERT(int(subArray.nrow()) == shape[0]);
     CPPUNIT_ASSERT(int(subArray.ncolumn()) == shape[1]);     
   }
};

} // namespace scimath

} // namespace askap

#endif // #ifndef PADDING_UTILS_TEST_H

