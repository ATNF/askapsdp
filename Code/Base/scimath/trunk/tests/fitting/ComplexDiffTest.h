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

#include <algorithm>
#include <set>

namespace conrad {

namespace scimath {

class ComplexDiffTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ComplexDiffTest);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST(testMultiply);
  CPPUNIT_TEST(testMultiplyVector);
  CPPUNIT_TEST(testConjugate);
  CPPUNIT_TEST(testParameterList);
  CPPUNIT_TEST(testParameterType);
  CPPUNIT_TEST_SUITE_END();
private:
  ComplexDiff f,g;

public:
  void setUp();
  void testAdd();
  void testMultiply();
  void testMultiplyVector();
  void testConjugate();
  void testParameterList();
  void testParameterType();
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

void ComplexDiffTest::testParameterList()
{
  ComplexDiff d = g*f+1+casa::Complex(0.,-2.);
  std::vector<std::string> buf;
  std::copy(d.begin(),d.end(),std::back_insert_iterator<std::vector<std::string> >(buf));
  CPPUNIT_ASSERT(buf[0] == "g1");
  CPPUNIT_ASSERT(buf[1] == "g2");
  CPPUNIT_ASSERT(buf.size() == 2);  
}

void ComplexDiffTest::testParameterType()
{
  ComplexDiff d("real",5);
  CPPUNIT_ASSERT(!g.isReal("g2"));
  CPPUNIT_ASSERT(!f.isReal("g1"));
  CPPUNIT_ASSERT(d.isReal("real"));
  ComplexDiff product = g*f*d;
  CPPUNIT_ASSERT(!product.isReal("g2"));
  CPPUNIT_ASSERT(!product.isReal("g1"));
  CPPUNIT_ASSERT(product.isReal("real"));
  
}

} // namespace scimath

} // namespace conrad

#endif // #ifndef COMPLEX_DIFF_TEST

