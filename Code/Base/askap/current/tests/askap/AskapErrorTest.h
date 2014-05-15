/// @file
/// 
/// @brief This file contains tests for macros defined in AskapError.
/// @details The tests cover the ASKAPASSERT and ASKAPCHECK macros
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
/// @author Malte Marquarding <Malte.Marquarding@csiro.au>

#ifndef ASKAP_ERROR_TEST_H
#define ASKAP_ERROR_TEST_H

#include <askap/AskapError.h>
#include <cppunit/extensions/HelperMacros.h>

namespace askap
{
  
  class AskapErrorTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE(AskapErrorTest);
    CPPUNIT_TEST(testIntAssert);
    CPPUNIT_TEST(testFloatAssert);
    CPPUNIT_TEST(testDoubleAssert);
    CPPUNIT_TEST(testIntCheck);
    CPPUNIT_TEST_SUITE_END();
    
  public:
    
    void testIntAssert() {
      CPPUNIT_ASSERT_NO_THROW(ASKAPASSERT(0==int(0)));
      int x = 0;
      CPPUNIT_ASSERT_THROW(ASKAPASSERT(x==1), askap::AssertError);
    }
    void testFloatAssert() {
      CPPUNIT_ASSERT_THROW(ASKAPASSERT(float(2)==1), askap::AssertError);
      CPPUNIT_ASSERT_NO_THROW(ASKAPASSERT(float(2)==2));
    }
    void testDoubleAssert() {
      CPPUNIT_ASSERT_THROW(ASKAPASSERT(double(3)==2), askap::AssertError);
    }
    
    void testIntCheck() {
      CPPUNIT_ASSERT_THROW(ASKAPCHECK(int(0)==1, "check0"), 
                           askap::CheckError);      
      try {
        int x=0;
        ASKAPCHECK(x==1, "check1");
      } catch (const askap::CheckError& ex) {
        CPPUNIT_ASSERT(std::string(ex.what()).find(std::string("check1 ('x==1' failed)"))
                != std::string::npos);
      }
    }

  };

} // namespace askap

#endif // #ifndef
