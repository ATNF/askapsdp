/// @file
/// 
/// @brief This file contains tests of generic normal equation
/// (the one, which implements normal equation in the very basic
/// form without any approximation).
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

#ifndef GENERIC_NORMAL_EQUATION_TEST_H
#define GENERIC_NORMAL_EQUATION_TEST_H


#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>


#include <fitting/GenericNormalEquations.h>
#include <fitting/DesignMatrix.h>


#include <cppunit/extensions/HelperMacros.h>

#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>


#include <askap/AskapError.h>

#include <boost/shared_ptr.hpp>
#include <algorithm>

namespace askap
{
  namespace scimath
  {

    class GenericNormalEquationsTest : public CppUnit::TestFixture 
    {
 
      CPPUNIT_TEST_SUITE(GenericNormalEquationsTest);
      CPPUNIT_TEST(testAddDesignMatrixScalar);
      CPPUNIT_TEST(testAddDesignMatrixNonScalar);
      CPPUNIT_TEST(testAddIndependentParameter);
      CPPUNIT_TEST(testMerge);
      CPPUNIT_TEST(testConstructorFromDesignMatrix);
      CPPUNIT_TEST_EXCEPTION(testNonConformanceError, askap::CheckError);
      CPPUNIT_TEST(testBlobStream);
      CPPUNIT_TEST_SUITE_END();

      private:
        boost::shared_ptr<GenericNormalEquations> itsNE;
        
      public:
        void setUp()
        { 
          itsNE.reset(new GenericNormalEquations);
        }
        
        void setUpNormalEquations()
        {
           
        }
        
        template<int N> void checkUnknowns(const char* correct[N]) {
           const std::vector<std::string> params = itsNE->unknowns();
           CPPUNIT_ASSERT(params.size() == N);
           for (int i=0;i<N;++i) {
                CPPUNIT_ASSERT(std::find(params.begin(),params.end(),correct[i]) != params.end());                
           }
        }
        
        void testAddDesignMatrixScalar()
        {
          const casa::uInt nData = 10;
          DesignMatrix dm;
          dm.addDerivative("Value0", casa::Matrix<casa::Double>(nData, 1, 1.0));
          dm.addDerivative("Value1", casa::Matrix<casa::Double>(nData, 1, 2.0));
          dm.addResidual(casa::Vector<casa::Double>(nData, -1.0), casa::Vector<double>(nData, 1.0));
          CPPUNIT_ASSERT(dm.nData() == nData);
          itsNE->add(dm);
          checkScalarResults(nData);
          const char* expected[2] = {"Value0","Value1"};
          checkUnknowns<2>(expected);
        }
        
        void checkScalarResults(casa::uInt nData) 
        {
          // checking that A^tA and A^B were calculated correctly
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value0", "Value0").shape() == 
               casa::IPosition(2,1,1));
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value0", "Value1").shape() == 
               casa::IPosition(2,1,1));
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value1", "Value1").shape() == 
               casa::IPosition(2,1,1));
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value1", "Value0").shape() == 
               casa::IPosition(2,1,1));
          CPPUNIT_ASSERT(fabs(itsNE->normalMatrix("Value0", "Value0")(0,0)-
                              double(nData))<1e-7);
          CPPUNIT_ASSERT(fabs(itsNE->normalMatrix("Value1", "Value1")(0,0)-
                              4.*nData)<1e-7);
          CPPUNIT_ASSERT(fabs(itsNE->normalMatrix("Value1", "Value0")(0,0)-
                              2.*nData)<1e-7);
          CPPUNIT_ASSERT(fabs(itsNE->normalMatrix("Value0", "Value1")(0,0)-
                              2.*nData)<1e-7);
          CPPUNIT_ASSERT(itsNE->dataVector("Value0").size() == 1);                    
          CPPUNIT_ASSERT(fabs(itsNE->dataVector("Value0")[0]+double(nData))<1e-7);
          CPPUNIT_ASSERT(itsNE->dataVector("Value1").size() == 1);                    
          CPPUNIT_ASSERT(fabs(itsNE->dataVector("Value1")[0]+2.*nData)<1e-7);             
        }
        
        static casa::Matrix<double> populateMatrix(casa::uInt nrow, casa::uInt ncol,
                                            const double *buf)
        {
          casa::Matrix<double> result(nrow,ncol,0.);
          for (casa::uInt row = 0; row<nrow; ++row) {
               for (casa::uInt col = 0; col<ncol; ++col,++buf) {
                    result(row,col) = *buf;
               }
          }
          return result;
        }
        
        static casa::Vector<double> populateVector(casa::uInt size, 
                                                   const double *buf)
        {
          casa::Vector<double> result(size,0.);
          std::copy(buf,buf+size,result.cbegin());
          return result;
        }
        
        void testAddDesignMatrixNonScalar()
        {
          const casa::uInt nData = 10;
          DesignMatrix dm;
          dm.addDerivative("ScalarValue", casa::Matrix<casa::Double>(nData, 1, 1.0));
          casa::Matrix<casa::Double> matrix(nData,2,2.);
          matrix.column(1) = -1.;
          dm.addDerivative("Value0", matrix);
          casa::Matrix<casa::Double> matrix2(nData,3,1.);
          matrix2.column(1) = 0.;
          matrix2.column(2) = -2.;
          dm.addDerivative("Value1", matrix2);
          dm.addResidual(casa::Vector<casa::Double>(nData, -1.0), casa::Vector<double>(nData, 1.0));
          CPPUNIT_ASSERT(dm.nData() == nData);
          itsNE->add(dm);
          
          // check that A^tA and A^tB were calculated correctly
          checkNonScalarResults(nData);
          // 
          const char* expected[3] = {"ScalarValue","Value0","Value1"};
          checkUnknowns<3>(expected);          
        }
        
        void checkNonScalarResults(casa::uInt nData) 
        {
          // check normal matrix first
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value0", "Value0").shape() == 
               casa::IPosition(2,2,2));
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value1", "Value1").shape() == 
               casa::IPosition(2,3,3));
          CPPUNIT_ASSERT(itsNE->normalMatrix("ScalarValue", 
               "ScalarValue").shape() == casa::IPosition(2,1,1));
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value0", "Value1").shape() == 
               casa::IPosition(2,2,3));
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value1", "Value0").shape() == 
               casa::IPosition(2,3,2));
          CPPUNIT_ASSERT(itsNE->normalMatrix("ScalarValue", "Value0").shape() == 
               casa::IPosition(2,1,2));
          CPPUNIT_ASSERT(itsNE->normalMatrix("ScalarValue", "Value1").shape() == 
               casa::IPosition(2,1,3));
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value0", "ScalarValue").shape() == 
               casa::IPosition(2,2,1));
          CPPUNIT_ASSERT(itsNE->normalMatrix("Value1", "ScalarValue").shape() == 
               casa::IPosition(2,3,1));
          
          const double m1[] = {4.,-2.,-2.,1.};
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Value0","Value0")-
                               populateMatrix(2,2,m1)*double(nData))<1e-7);
          const double m2[] = {1.,0.,-2.,0.,0.,0.,-2.,0.,4.};
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Value1","Value1")-
                               populateMatrix(3,3,m2)*double(nData))<1e-7);
          const double m3[] = {2.,0.,-4.,-1.,0.,2.};
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Value0","Value1")-
                               populateMatrix(2,3,m3)*double(nData))<1e-7);
          const double m4[] = {2.,-1.,0.,0.,-4.,2.};
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Value1","Value0")-
                               populateMatrix(3,2,m4)*double(nData))<1e-7);
          const double m5[] = {2.,-1.};
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("ScalarValue","Value0")-
                               populateMatrix(1,2,m5)*double(nData))<1e-7);
          const double m6[] = {1.,0.,-2.};
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("ScalarValue","Value1")-
                               populateMatrix(1,3,m6)*double(nData))<1e-7);
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Value0","ScalarValue")-
                               populateMatrix(2,1,m5)*double(nData))<1e-7);
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Value1","ScalarValue")-
                               populateMatrix(3,1,m6)*double(nData))<1e-7);
          CPPUNIT_ASSERT(fabs(itsNE->normalMatrix("ScalarValue",
                         "ScalarValue")(0,0) - double(nData))<1e-7);
          // check right-hand side part
          CPPUNIT_ASSERT(itsNE->dataVector("ScalarValue").size() == 1);                    
          CPPUNIT_ASSERT(fabs(itsNE->dataVector("ScalarValue")[0]+
                              double(nData))<1e-7);
          CPPUNIT_ASSERT(norm(itsNE->dataVector("Value0")+populateVector(2,m5)*
                              double(nData))<1e-7);               
          CPPUNIT_ASSERT(norm(itsNE->dataVector("Value1")+populateVector(3,m6)*
                              double(nData))<1e-7);               
        }
        
        void checkIndependentResults()
        {
          // check independence                     
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Independent",
                               "ScalarValue"))<1e-7);
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("ScalarValue",
                               "Independent"))<1e-7);
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Independent", "Value0"))<1e-7);
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Independent", "Value1"))<1e-7);
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Value0", "Independent"))<1e-7);
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Value1", "Independent"))<1e-7);
        }
        
        void testAddIndependentParameter()
        {
          testAddDesignMatrixNonScalar();
          const double m[] = {0.,1.,1.,0.}; // matrix for the new parameter
          const double v[] = {2.,-3.}; // data vector for the new parameter
          itsNE->add("Independent", populateMatrix(2,2,m), populateVector(2,v));
          
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Independent","Independent")-
                               populateMatrix(2,2,m))<1e-7);
          CPPUNIT_ASSERT(norm(itsNE->dataVector("Independent")-
                               populateVector(2,v))<1e-7);
          // check for independence
          checkIndependentResults();
          // check that other matrix elements are intact (nData = 10)
          checkNonScalarResults(10);  
          //
          const char* expected[4] = {"Independent","ScalarValue","Value0","Value1"};
          checkUnknowns<4>(expected);          
        }
        
        void testMerge()
        {
          testAddIndependentParameter();
          boost::shared_ptr<GenericNormalEquations> bufNE = itsNE;
          itsNE.reset(new GenericNormalEquations);
          testAddDesignMatrixNonScalar();
          itsNE->merge(*bufNE);
          
          const double m[] = {0.,1.,1.,0.}; // matrix for independent parameter
          const double v[] = {2.,-3.}; // data vector for independent parameter
          CPPUNIT_ASSERT(norm1(itsNE->normalMatrix("Independent","Independent")-
                               populateMatrix(2,2,m))<1e-7);
          CPPUNIT_ASSERT(norm(itsNE->dataVector("Independent")-
                               populateVector(2,v))<1e-7);
          // check for independence
          checkIndependentResults();
          // check other matrix elements. nData = 20 because each data
          // point has been added twice due to merging and all matrix elements
          // are expected to be scaled appropriately.
          checkNonScalarResults(20);  
          //
          const char* expected[4] = {"Independent","ScalarValue","Value0","Value1"};
          checkUnknowns<4>(expected);          
        }
        
        void testConstructorFromDesignMatrix()
        {
          testAddDesignMatrixScalar();
          const casa::uInt nData = 20;
          DesignMatrix dm;
          dm.addDerivative("Value0", casa::Matrix<casa::Double>(nData, 1, 1.0));
          dm.addDerivative("Value1", casa::Matrix<casa::Double>(nData, 1, 2.0));
          dm.addResidual(casa::Vector<casa::Double>(nData, -1.0), casa::Vector<double>(nData, 1.0));
          CPPUNIT_ASSERT(dm.nData() == nData);
          GenericNormalEquations gne(dm);
          itsNE->merge(gne);
          checkScalarResults(30); // we add more data points, elements should scale
          itsNE->reset();
          itsNE->merge(gne);
          checkScalarResults(20); // now only this design matrix is important
          itsNE=boost::dynamic_pointer_cast<GenericNormalEquations>(gne.clone());
          CPPUNIT_ASSERT(itsNE);
          checkScalarResults(20); // the same, but here we're checking constructor
                                  // only
          //
          const char* expected[2] = {"Value0","Value1"};
          checkUnknowns<2>(expected);          
        }
        
        void testNonConformanceError()
        {
          testAddDesignMatrixScalar();
          testAddDesignMatrixNonScalar();
        }

        void testBlobStream()
        {
          testAddDesignMatrixNonScalar();
          LOFAR::BlobString bstr(false);
          LOFAR::BlobOBufString bob(bstr);
          LOFAR::BlobOStream bos(bob);
          
          bos<<*itsNE;
          
          itsNE->reset();
          
          LOFAR::BlobIBufString bib(bstr);
          LOFAR::BlobIStream bis(bib);
          bis>>*itsNE;
          
          checkNonScalarResults(10);
          //
          const char* expected[3] = {"ScalarValue","Value0","Value1"};
          checkUnknowns<3>(expected);          
        }
    };

  }
}

#endif // #ifndef GENERIC_NORMAL_EQUATION_TEST_H

