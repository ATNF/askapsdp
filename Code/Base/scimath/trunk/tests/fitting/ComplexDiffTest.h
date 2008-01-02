/// @file
/// 
/// @brief Tests of ComplexDiff autodifferentiation class
/// @details See ComplexDiff for description of what this class  
/// is supposed to do. This file contains appropriate unit tests.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef COMPLEX_DIFF_TEST
#define COMPLEX_DIFF_TEST

#include <fitting/ComplexDiff.h>

#include <cppunit/extensions/HelperMacros.h>

#include <conrad/ConradError.h>

namespace conrad {

namespace scimath {

class ComplexDiffTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ComplexDiffTest);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST_SUITE_END();
private:
  ComplexDiff f,g;

public:
  void setUp();
  void testAdd();
};

void ComplexDiffTest::setUp() 
{
  f = ComplexDiff("g1",casa::Complex(35.,-15.));
  g = ComplexDiff("g2",casa::Complex(-35.,15.));
}

void ComplexDiffTest::testAdd()
{
  f+=g;
  CPPUNIT_ASSERT(abs(f.value())<1e-7);
  CPPUNIT_ASSERT(abs(f.derivRe("g1")-casa::Complex(1.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(f.derivRe("g2")-casa::Complex(1.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(f.derivIm("g1")-casa::Complex(0.,1.))<1e-7);
  CPPUNIT_ASSERT(abs(f.derivIm("g2")-casa::Complex(0.,1.))<1e-7);
  g+=f;
  CPPUNIT_ASSERT(abs(g.derivRe("g1")-casa::Complex(1.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivIm("g1")-casa::Complex(0.,1.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivRe("g2")-casa::Complex(2.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivIm("g2")-casa::Complex(0.,2.))<1e-7);
  
  //std::cout<<g.derivRe("g2")<<" "<<g.derivIm("g2")<<std::endl;
}


} // namespace scimath

} // namespace conrad

#endif // #ifndef COMPLEX_DIFF_TEST

