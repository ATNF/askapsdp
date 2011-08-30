/// @file
/// 
/// @brief Tests of ComplexDiff autodifferentiation class
/// @details See ComplexDiff for description of what this class  
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

#ifndef COMPLEX_DIFF_MATRIX_TEST
#define COMPLEX_DIFF_MATRIX_TEST

// casa includes
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/MatrixMath.h>
#include <scimath/Mathematics/MatrixMathLA.h>


// own includes
#include <fitting/ComplexDiff.h>
#include <fitting/ComplexDiffMatrix.h>

#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapError.h>

// stl includes

#include <algorithm>
#include <set>
#include <string>

namespace askap {

namespace scimath {

class ComplexDiffMatrixTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ComplexDiffMatrixTest);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST(testMultiply);
  CPPUNIT_TEST(testMatrixMultiply);
  CPPUNIT_TEST(testMultiplyByScalar);
  CPPUNIT_TEST(testParameterList);
  CPPUNIT_TEST(testCreateFromVector);  
  CPPUNIT_TEST(testCreateFromMatrix); 
  CPPUNIT_TEST(testReuse);   
  CPPUNIT_TEST_SUITE_END();
private:
  ComplexDiff f,g;

public:
  void setUp();
  void testAdd();
  void testMultiply();
  void testMatrixMultiply();
  void testMultiplyByScalar();
  void testCreateFromVector();
  void testCreateFromMatrix();
  void testParameterList();
  void testReuse();
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

void ComplexDiffMatrixTest::testMatrixMultiply()
{
  // this test is explicitly intended to test matrix multiply rather than
  // carriage of derivatives (which is reasonably tested in ComplexDiff test)
  
  casa::Matrix<casa::Complex> M(4,4);
  M(0,0) = casa::Complex(1.234,-0.01);   M(0,1) = casa::Complex(0.234,0.31);
  M(0,2) = casa::Complex(-0.74,-0.023);  M(0,3) = casa::Complex(0.0004,0.03);
  M(1,0) = casa::Complex(-0.0154,casa::C::pi/10.); M(1,1) = casa::Complex(2.4,-1.3);
  M(1,2) = casa::Complex(0.04,-0.0123); M(1,3) = casa::Complex(2.9e-4,0.089);
  M(2,0) = casa::Complex(1.0,-0.42); M(2,1) = casa::Complex(-0.097,-0.067);
  M(2,2) = casa::Complex(3.4,0.8); M(2,3) = casa::Complex(-0.43,0.33);
  M(3,0) = casa::Complex(-0.09,-0.038); M(3,1) = casa::Complex(-0.74,0.023);
  M(3,2) = casa::Complex(0.,0.); M(3,3) = casa::Complex(1.0,0.0);
  // fill CDM
  ComplexDiffMatrix cdm(M);     
  // compute inverse
  casa::Matrix<casa::Complex> reciprocal(M.nrow(),M.ncolumn());
  casa::Complex det = 0.;
  casa::invert(reciprocal,det,M);
  CPPUNIT_ASSERT(abs(det)>1e-5);
  CPPUNIT_ASSERT_EQUAL(M.nrow(),reciprocal.nrow());
  CPPUNIT_ASSERT_EQUAL(M.ncolumn(),reciprocal.ncolumn());
  CPPUNIT_ASSERT_EQUAL(M.nrow(),casa::uInt(cdm.nRow()));
  CPPUNIT_ASSERT_EQUAL(M.ncolumn(),casa::uInt(cdm.nColumn()));
  // fill another CDM
  ComplexDiffMatrix cdm2(reciprocal);
  CPPUNIT_ASSERT_EQUAL(M.nrow(),casa::uInt(cdm2.nRow()));
  CPPUNIT_ASSERT_EQUAL(M.ncolumn(),casa::uInt(cdm2.nColumn()));
  // compute the product
  ComplexDiffMatrix cdm3 = cdm * cdm2;
  CPPUNIT_ASSERT(cdm3.paramBegin() == cdm3.paramEnd());
  for (size_t i = 0; i<cdm3.nRow(); ++i) {
       for (size_t j = 0; j<cdm3.nColumn(); ++j) {
            const casa::Complex val = cdm3(i,j).value();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(i == j ? 1. : 0., real(val), 1e-6);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0., imag(val), 1e-6);            
       }
  } 
  
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
  
  ComplexDiffMatrix cdm2 = g * vec;
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
  
  ComplexDiffMatrix cdm2 = g * matr;
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

void ComplexDiffMatrixTest::testReuse()
{
  ComplexDiffMatrix cdm(2,2,f);
  cdm(0,0) = g;
  ComplexDiffMatrix cdm2(cdm);
  cdm2(1,1) = g*f;
  cdm2.set(g);
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


} // namespace scimath

} // namespace askap

#endif // #ifndef COMPLEX_DIFF_MATRIX_TEST

