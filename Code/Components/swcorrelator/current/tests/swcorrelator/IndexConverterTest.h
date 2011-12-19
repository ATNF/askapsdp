/// @file
///
/// @brief Test of the index converter helper class
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#ifndef ASKAP_SWCORRELATOR_INDEX_CONVERTER_TEST_H
#define ASKAP_SWCORRELATOR_INDEX_CONVERTER_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <swcorrelator/IndexConverter.h>

#include <string>

namespace askap {

namespace swcorrelator {

class IndexConverterTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(IndexConverterTest);
  CPPUNIT_TEST(testVoidConversion);
  CPPUNIT_TEST(test1Elem);
  CPPUNIT_TEST(testManyElements);
  CPPUNIT_TEST_SUITE_END();
public:
  void testVoidConversion() {
     IndexConverter ic;
     for (int index=0; index<30; ++index) {
         CPPUNIT_ASSERT_EQUAL(index, ic(index));
     }
     ic.add("");
     for (int index=0; index<30; ++index) {
         CPPUNIT_ASSERT_EQUAL(index, ic(index));
     }     
  }
  
  void test1Elem() {
     IndexConverter ic("1:0");
     for (int index=0; index<30; ++index) {
         CPPUNIT_ASSERT_EQUAL(index == 1 ? 0 : -1, ic(index));
     } 
     IndexConverter ic2("2:8");
     for (int index=0; index<30; ++index) {
         CPPUNIT_ASSERT_EQUAL(index == 2 ? 8 : -1, ic2(index));
     }               
  }

  void testManyElements() {
     IndexConverter ic;
     for (int index=0; index<30; ++index) {
         CPPUNIT_ASSERT_EQUAL(index, ic(index));
     }
     ic.add("1:2, 3:0,5:1");
     for (int index=0; index<30; ++index) {
          if ((index != 1) && (index != 3) && (index != 5)) {
              CPPUNIT_ASSERT_EQUAL(-1, ic(index));
          }
          CPPUNIT_ASSERT_EQUAL(2, ic(1));
          CPPUNIT_ASSERT_EQUAL(0, ic(3));
          CPPUNIT_ASSERT_EQUAL(1, ic(5));          
     }          
  }

};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_INDEX_CONVERTER_TEST_H

