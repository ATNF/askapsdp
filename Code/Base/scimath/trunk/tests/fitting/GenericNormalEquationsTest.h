/// @file
/// 
/// @brief This file contains tests of generic normal equation
/// (the one, which implements normal equation in the very basic
/// form without any approximation).
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef GENERIC_NORMAL_EQUATION_TEST_H
#define GENERIC_NORMAL_EQUATION_TEST_H


#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>


#include <fitting/GenericNormalEquations.h>
#include <fitting/DesignMatrix.h>

#include <fitting/NormalEquations.h>

#include <cppunit/extensions/HelperMacros.h>

#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>


#include <conrad/ConradError.h>

#include <boost/shared_ptr.hpp>

namespace conrad
{
  namespace scimath
  {

    class GenericNormalEquationsTest : public CppUnit::TestFixture 
    {
 
      CPPUNIT_TEST_SUITE(GenericNormalEquationsTest);
      CPPUNIT_TEST(testAddDesignMatrixScalar);
      CPPUNIT_TEST(testAddDesignMatrixNonScalar);
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

        
        void testAddDesignMatrixScalar()
        {
          const casa::uInt nData = 10;
          DesignMatrix dm;
          dm.addDerivative("Value0", casa::Matrix<casa::Double>(nData, 1, 1.0));
          dm.addDerivative("Value1", casa::Matrix<casa::Double>(nData, 1, 2.0));
          dm.addResidual(casa::Vector<casa::Double>(nData, -1.0), casa::Vector<double>(nData, 1.0));
          CPPUNIT_ASSERT(dm.nData() == nData);
          itsNE->add(dm);
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
          dm.addResidual(casa::Vector<casa::Double>(nData, 1.0), casa::Vector<double>(nData, 1.0));
          CPPUNIT_ASSERT(dm.nData() == nData);
          itsNE->add(dm);
          
          // check that A^tA and A^tB were calculated correctly
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
        }

    };

  }
}

#endif // #ifndef GENERIC_NORMAL_EQUATION_TEST_H

