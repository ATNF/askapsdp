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

// casa includes
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>

// own includes
#include <fitting/ComplexDiff.h>
#include <fitting/ComplexDiffMatrix.h>

#include <cppunit/extensions/HelperMacros.h>

#include <conrad/ConradError.h>

// stl includes

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
  CPPUNIT_TEST(testMultiplyByScalar);
  CPPUNIT_TEST(testParameterList);
  CPPUNIT_TEST(testCreateFromVector);  
  CPPUNIT_TEST(testCreateFromMatrix);  
  CPPUNIT_TEST_SUITE_END();
private:
  ComplexDiff f,g;

public:
  void setUp();
  void testAdd();
  void testMultiply();
  void testMultiplyByScalar();
  void testCreateFromVector();
  void testCreateFromMatrix();
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

void ComplexDiffMatrixTest::testMultiplyByScalar()
{
  ComplexDiffMatrix cdm(2,2,f);
  cdm(0,0)=g;
  
  ComplexDiffMatrix cdm3 = cdm * g;
  
  CPPUNIT_ASSERT(abs(cdm3(0,0).value()-casa::Complex(1000.,-1050.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivRe("g1")-casa::Complex(0.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivIm("g1")-casa::Complex(0.,0.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivRe("g2")-casa::Complex(-70.,30.))<1e-7);
  CPPUNIT_ASSERT(abs(cdm3(0,0).derivIm("g2")-casa::Complex(-30.,-70.))<1e-7);

  for (size_t i=1; i<4; ++i) {  
       CPPUNIT_ASSERT(abs(cdm3(i/2,i%2).value()-casa::Complex(-1000.,1050.))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3(i/2,i%2).derivRe("g1")-casa::Complex(-35.,15.))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3(i/2,i%2).derivIm("g1")-casa::Complex(-15.,-35.))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3(i/2,i%2).derivRe("g2")-casa::Complex(35.,-15.))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3(i/2,i%2).derivIm("g2")-casa::Complex(15.,35.))<1e-7);
  }
  
  ComplexDiffMatrix cdm2 = g * cdm;
  
  ComplexDiffMatrix::const_iterator cdm3It = cdm3.begin();
  ComplexDiffMatrix::const_iterator cdm2It = cdm2.begin();
  for (; cdm3It != cdm3.end(); ++cdm3It, ++cdm2It) {
       CPPUNIT_ASSERT(cdm2It != cdm2.end());
       CPPUNIT_ASSERT(abs(cdm3It->value()-cdm2It->value())<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivRe("g1")-cdm2It->derivRe("g1"))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivIm("g1")-cdm2It->derivIm("g1"))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivRe("g2")-cdm2It->derivRe("g2"))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivIm("g2")-cdm2It->derivIm("g2"))<1e-7);       
  }
}

void ComplexDiffMatrixTest::testCreateFromVector()
{
  const size_t nelem = 5;
  casa::Vector<casa::Complex> vec(nelem, casa::Complex(10., -5.));
  ComplexDiffMatrix cdm = vec;
  CPPUNIT_ASSERT(cdm.nRow() == nelem);
  CPPUNIT_ASSERT(cdm.nColumn() == 1);
  for (size_t i=0; i<nelem; ++i) {
       CPPUNIT_ASSERT(abs(cdm(i,0).value()-casa::Complex(10.,-5.))<1e-7);   
  }
  
  ComplexDiffMatrix cdm2 = g * ComplexDiffMatrix(vec);
  ComplexDiffMatrix cdm3 = cdm * g;
  
  ComplexDiffMatrix::const_iterator cdm3It = cdm3.begin();
  ComplexDiffMatrix::const_iterator cdm2It = cdm2.begin();
  for (; cdm3It != cdm3.end(); ++cdm3It, ++cdm2It) {
       CPPUNIT_ASSERT(cdm2It != cdm2.end());
       CPPUNIT_ASSERT(abs(cdm3It->value()-cdm2It->value())<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivRe("g1")-cdm2It->derivRe("g1"))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivIm("g1")-cdm2It->derivIm("g1"))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivRe("g2")-cdm2It->derivRe("g2"))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivIm("g2")-cdm2It->derivIm("g2"))<1e-7);       
  }  
}

void ComplexDiffMatrixTest::testCreateFromMatrix()
{
  const size_t nrow = 5;
  const size_t ncol = 10;
  
  casa::Matrix<casa::Complex> matr(nrow,ncol, casa::Complex(10., -5.));
  ComplexDiffMatrix cdm = matr;
  CPPUNIT_ASSERT(cdm.nRow() == nrow);
  CPPUNIT_ASSERT(cdm.nColumn() == ncol);
  for (size_t i=0; i<nrow; ++i) {
       for (size_t j=0; j<ncol; ++j) {
           CPPUNIT_ASSERT(abs(cdm(i,j).value()-casa::Complex(10.,-5.))<1e-7);   
       }
  }
  
  ComplexDiffMatrix cdm2 = g * ComplexDiffMatrix(matr);
  ComplexDiffMatrix cdm3 = cdm * g;
  
  ComplexDiffMatrix::const_iterator cdm3It = cdm3.begin();
  ComplexDiffMatrix::const_iterator cdm2It = cdm2.begin();
  for (; cdm3It != cdm3.end(); ++cdm3It, ++cdm2It) {
       CPPUNIT_ASSERT(cdm2It != cdm2.end());
       CPPUNIT_ASSERT(abs(cdm3It->value()-cdm2It->value())<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivRe("g1")-cdm2It->derivRe("g1"))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivIm("g1")-cdm2It->derivIm("g1"))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivRe("g2")-cdm2It->derivRe("g2"))<1e-7);
       CPPUNIT_ASSERT(abs(cdm3It->derivIm("g2")-cdm2It->derivIm("g2"))<1e-7);       
  }  
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

