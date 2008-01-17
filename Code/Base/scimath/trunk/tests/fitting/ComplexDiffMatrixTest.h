/// @file
/// 
/// @brief Tests of ComplexDiff autodifferentiation class
/// @details See ComplexDiff for description of what this class  
/// is supposed to do. This file contains appropriate unit tests.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef COMPLEX_DIFF_MATRIX_TEST
#define COMPLEX_DIFF_MATRIX_TEST

#include <fitting/ComplexDiff.h>
#include <fitting/ComplexDiffMatrix.h>

#include <cppunit/extensions/HelperMacros.h>

#include <conrad/ConradError.h>

#include <algorithm>
#include <set>
#include <string>

namespace conrad {

namespace scimath {

class ComplexDiffMatrixTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ComplexDiffMatrixTest);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST(testMultiply);
  CPPUNIT_TEST(testParameterList);  
  CPPUNIT_TEST_SUITE_END();
private:
  ComplexDiff f,g;

public:
  void setUp();
  void testAdd();
  void testMultiply();
  void testParameterList();
};

void ComplexDiffMatrixTest::setUp() 
{
  f = ComplexDiff("g1",casa::Complex(35.,-15.));
  g = ComplexDiff("g2",casa::Complex(-35.,15.));
}

void ComplexDiffMatrixTest::testAdd()
{
  ComplexDiffMatrix cdm(2,2,f);
  cdm(0,0)=g;
  ComplexDiffMatrix cdm2(2,2,g);
  cdm2(1,1)=f;
  
  ComplexDiffMatrix cdm3 = cdm + cdm2;
  
  CPPUNIT_ASSERT(abs(cdm3(0,0).value()-casa::Complex(-70.,30.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivRe("g1")-casa::Complex(0.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivIm("g1")-casa::Complex(0.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivRe("g2")-casa::Complex(2.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivIm("g2")-casa::Complex(0.,2.))<1e-7);
  
  CPPUNIT_ASSERT(abs(cdm3(0,1).value()-casa::Complex(0.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,1).derivRe("g1")-casa::Complex(1.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,1).derivIm("g1")-casa::Complex(0.,1.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,1).derivRe("g2")-casa::Complex(1.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,1).derivIm("g2")-casa::Complex(0.,1.))<1e-7);
  
  CPPUNIT_ASSERT(abs(cdm3(1,0).value()-casa::Complex(0.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,0).derivRe("g1")-casa::Complex(1.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,0).derivIm("g1")-casa::Complex(0.,1.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,0).derivRe("g2")-casa::Complex(1.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,0).derivIm("g2")-casa::Complex(0.,1.))<1e-7);
  
  CPPUNIT_ASSERT(abs(cdm3(1,1).value()-casa::Complex(70.,-30.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,1).derivRe("g1")-casa::Complex(2.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,1).derivIm("g1")-casa::Complex(0.,2.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,1).derivRe("g2")-casa::Complex(0.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,1).derivIm("g2")-casa::Complex(0.,0.))<1e-7);
  
}

void ComplexDiffMatrixTest::testMultiply()
{
  ComplexDiffMatrix cdm(2,2,f);
  cdm(0,0)=g;
  ComplexDiffMatrix cdm2(2,2,g);
  cdm2(1,1)=f;
  
  ComplexDiffMatrix cdm3 = cdm * cdm2;
  
  CPPUNIT_ASSERT(abs(cdm3(0,0).value()-casa::Complex(0.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivRe("g1")-casa::Complex(-35.,15.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivIm("g1")-casa::Complex(-15.,-35.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivRe("g2")-casa::Complex(-35.,15.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivIm("g2")-casa::Complex(-15.,-35.))<1e-7);
  
  CPPUNIT_ASSERT(abs(cdm3(0,1).value()-casa::Complex(2000.,-2100.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,1).derivRe("g1")-casa::Complex(70.,-30.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,1).derivIm("g1")-casa::Complex(30.,70.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,1).derivRe("g2")-casa::Complex(-70.,30.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,1).derivIm("g2")-casa::Complex(-30.,-70.))<1e-7);
  
  CPPUNIT_ASSERT(abs(cdm3(1,0).value()-casa::Complex(-2000.,2100.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,0).derivRe("g1")-casa::Complex(-70.,30.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,0).derivIm("g1")-casa::Complex(-30.,-70.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,0).derivRe("g2")-casa::Complex(70.,-30.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,0).derivIm("g2")-casa::Complex(30.,70.))<1e-7);
  
  CPPUNIT_ASSERT(abs(cdm3(1,1).value()-casa::Complex(0.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,1).derivRe("g1")-casa::Complex(35.,-15.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,1).derivIm("g1")-casa::Complex(15.,35.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,1).derivRe("g2")-casa::Complex(35.,-15.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(1,1).derivIm("g2")-casa::Complex(15.,35.))<1e-7);  
}

void ComplexDiffMatrixTest::testParameterList()
{
  ComplexDiffMatrix cdm(2,2,f);
  cdm(0,0)=g;
  ComplexDiffMatrix cdm2(2,2,g);
  cdm2(1,1)=f;
  
  ComplexDiffMatrix cdm3 = cdm * cdm2;
  std::set<std::string> params(cdm3.paramBegin(),cdm3.paramEnd());
  CPPUNIT_ASSERT(params.count("g1") == 1);
  CPPUNIT_ASSERT(params.count("g2") == 1);
  CPPUNIT_ASSERT(params.size() == 2);
}


} // namespace scimath

} // namespace conrad

#endif // #ifndef COMPLEX_DIFF_MATRIX_TEST

