/// @file
/// 
/// @brief This file contains tests for macros defined in ConradError.
/// @details The tests cover the CONRADASSERT and CONRADCHECK macros
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Malte Marquarding <Malte.Marquarding@csiro.au>

#ifndef CONRAD_ERROR_TEST_H
#define CONRAD_ERROR_TEST_H

#include <askap/ConradError.h>
#include <cppunit/extensions/HelperMacros.h>

namespace conrad
{
  
  class ConradErrorTest : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE(ConradErrorTest);
    CPPUNIT_TEST(testIntAssert);
    CPPUNIT_TEST(testFloatAssert);
    CPPUNIT_TEST(testDoubleAssert);
    CPPUNIT_TEST(testIntCheck);
    CPPUNIT_TEST_SUITE_END();
    
  public:
    
    void testIntAssert() {
      CPPUNIT_ASSERT_NO_THROW(CONRADASSERT(0==int(0)));
      int x = 0;
      CPPUNIT_ASSERT_THROW(CONRADASSERT(x==1), conrad::AssertError);
    }
    void testFloatAssert() {
      CPPUNIT_ASSERT_THROW(CONRADASSERT(float(2)==1), conrad::AssertError);
      CPPUNIT_ASSERT_NO_THROW(CONRADASSERT(float(2)==2));
    }
    void testDoubleAssert() {
      CPPUNIT_ASSERT_THROW(CONRADASSERT(double(3)==2), conrad::AssertError);
    }
    
    void testIntCheck() {
      CPPUNIT_ASSERT_THROW(CONRADCHECK(int(0)==1, "check0"), 
                           conrad::CheckError);      
      try {
        int x=0;
        CONRADCHECK(x==1, "check1");
      } catch (const conrad::CheckError& ex) {
        CPPUNIT_ASSERT(std::string(ex.what()).find(std::string("check1 ('x==1' failed)"))
                != std::string::npos);
      }
    }

  };

} // namespace conrad

#endif // #ifndef
