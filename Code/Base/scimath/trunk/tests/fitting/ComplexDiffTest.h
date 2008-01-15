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
  CPPUNIT_TEST(testMultiply);
  CPPUNIT_TEST(testMultiplyVector);
  CPPUNIT_TEST(testConjugate);
  CPPUNIT_TEST_SUITE_END();
private:
  ComplexDiff f,g;

public:
  void setUp();
  void testAdd();
  void testMultiply();
  void testMultiplyVector();
  void testConjugate();
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
  CPPUNIT_ASSERT(abs(g.value()-casa::Complex(-35.,15.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivRe("g1")-casa::Complex(1.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivIm("g1")-casa::Complex(0.,1.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivRe("g2")-casa::Complex(2.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivIm("g2")-casa::Complex(0.,2.))<1e-7);
  
  ComplexDiff d = g+f+1+casa::Complex(0.,-2.);
  
  CPPUNIT_ASSERT(abs(d.value()-casa::Complex(-34.,13.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivRe("g1")-casa::Complex(2.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivIm("g1")-casa::Complex(0.,2.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivRe("g2")-casa::Complex(3.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivIm("g2")-casa::Complex(0.,3.))<1e-7);  
}

void ComplexDiffTest::testMultiply()
{
  ComplexDiff d = g*f;
  CPPUNIT_ASSERT(abs(d.value()-casa::Complex(-1000.,1050.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivRe("g1")-casa::Complex(-35.,15.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivIm("g1")-casa::Complex(-15.,-35.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivRe("g2")-casa::Complex(35.,-15.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivIm("g2")-casa::Complex(15.,35.))<1e-7);
  
  g*=f;
  
  CPPUNIT_ASSERT(abs(g.value()-casa::Complex(-1000.,1050.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivRe("g1")-casa::Complex(-35.,15.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivIm("g1")-casa::Complex(-15.,-35.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivRe("g2")-casa::Complex(35.,-15.))<1e-7);
  CPPUNIT_ASSERT(abs(g.derivIm("g2")-casa::Complex(15.,35.))<1e-7);
  
  d = g*casa::Complex(0.,1.);

  CPPUNIT_ASSERT(abs(d.value()-casa::Complex(-1050.,-1000.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivRe("g1")-casa::Complex(-15,-35.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivIm("g1")-casa::Complex(35.,-15.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivRe("g2")-casa::Complex(15,35.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivIm("g2")-casa::Complex(-35.,15.))<1e-7);
}

void ComplexDiffTest::testMultiplyVector()
{
  casa::Vector<casa::Complex> vec(10.,casa::Complex(0.,-2.));
  casa::Vector<ComplexDiff> cdVec = vec * f;
  
  for (casa::uInt i = 0; i< vec.nelements(); ++i) {
       CONRADASSERT(i < cdVec.nelements());
       const ComplexDiff &d = cdVec[i]; 
       CPPUNIT_ASSERT(abs(d.value()-casa::Complex(-30.,-70.))<1e-7);
       CPPUNIT_ASSERT(abs(d.derivRe("g1")-casa::Complex(0,-2.))<1e-7);
       CPPUNIT_ASSERT(abs(d.derivIm("g1")-casa::Complex(2.,0.))<1e-7);
  }
  
  cdVec = g * vec;

  for (casa::uInt i = 0; i< vec.nelements(); ++i) {
       CONRADASSERT(i < cdVec.nelements());
       const ComplexDiff &d = cdVec[i]; 
       CPPUNIT_ASSERT(abs(d.value()-casa::Complex(30.,70.))<1e-7);
       CPPUNIT_ASSERT(abs(d.derivRe("g2")-casa::Complex(0,-2.))<1e-7);
       CPPUNIT_ASSERT(abs(d.derivIm("g2")-casa::Complex(2.,0.))<1e-7);
  } 
  
  cdVec*= f;  

  for (casa::uInt i = 0; i< vec.nelements(); ++i) {
       CONRADASSERT(i < cdVec.nelements());
       const ComplexDiff &d = cdVec[i]; 
       CPPUNIT_ASSERT(abs(d.value()-casa::Complex(2100.,2000))<1e-7);
       CPPUNIT_ASSERT(abs(d.derivRe("g1")-casa::Complex(30.,70.))<1e-7);
       CPPUNIT_ASSERT(abs(d.derivIm("g1")-casa::Complex(-70.,30.))<1e-7);
       CPPUNIT_ASSERT(abs(d.derivRe("g2")-casa::Complex(-30.,-70.))<1e-7);
       CPPUNIT_ASSERT(abs(d.derivIm("g2")-casa::Complex(70.,-30.))<1e-7);
  } 
}

void ComplexDiffTest::testConjugate()
{
  ComplexDiff d = conj(g);
  CPPUNIT_ASSERT(abs(d.value()-casa::Complex(-35.,-15.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivRe("g2")-casa::Complex(1.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(d.derivIm("g2")-casa::Complex(0.,-1.))<1e-7);
}

} // namespace scimath

} // namespace conrad

#endif // #ifndef COMPLEX_DIFF_TEST

