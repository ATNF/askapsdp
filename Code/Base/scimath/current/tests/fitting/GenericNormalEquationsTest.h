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
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/PolXProducts.h>


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
      CPPUNIT_TEST(testAddProduct);
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
                        
        // testing addition of normal equations based on a product of
        // complex diff matrix and a vector presented by cross-products
        void testAddProduct() {
           ComplexDiffMatrix cdm(2,2);
           // some dummy parameters, simple equation sum_j(gij*vj)=y_i
           cdm(0,0) = ComplexDiff("g11", casa::Complex(1.1,-1.1));
           cdm(0,1) = ComplexDiff("g12", casa::Complex(1.2,-1.2));
           cdm(1,0) = ComplexDiff("g21", casa::Complex(2.1,-2.1));
           cdm(1,1) = ComplexDiff("g22", casa::Complex(2.2,-2.2));
           
           // vector to multiply
           casa::Vector<casa::Complex> vec(cdm.nColumn());
           vec[0] = casa::Complex(10.,1.);
           vec[1] = casa::Complex(1.,-10.);
           
           // measured data
           casa::Vector<casa::Complex> measured(vec.nelements());
           
           // vector for actual design equations
           ComplexDiffMatrix cdv(cdm.nRow(),1,0.);
           for (casa::uInt row = 0; row<cdv.nRow(); ++row) {
                for (casa::uInt col = 0; col<cdm.nColumn(); ++col) {
                    cdv[row] += cdm(row,col) * vec[col];
                }
                measured[row] = cdv[row].value();
           }
           // perturb the measured data a bit to have a non-zero data vector
           measured[0] += casa::Complex(0.01,-0.01);
           measured[0] += casa::Complex(-0.02,0.02);
           // use the standard approach via the design matrix
           DesignMatrix dm;
           dm.addModel(cdv,measured.reform(casa::IPosition(2,measured.nelements(),1)),casa::Matrix<double>(measured.nelements(),1,1.));
           GenericNormalEquations ne1;
           ne1.add(dm);
           std::vector<std::string> unknowns1 = ne1.unknowns();
           CPPUNIT_ASSERT_EQUAL(size_t(4), unknowns1.size());
           
           // now use the direct approach to obtain normal equations
                      
           // 1D buffer initialised to zero
           PolXProducts pxp(vec.nelements(), casa::IPosition(), true);
           for (casa::uInt pol1 = 0; pol1<vec.nelements(); ++pol1) {
                for (casa::uInt pol2 = 0; pol2<vec.nelements(); ++pol2) {
                     pxp.addModelMeasProduct(pol1,pol2, conj(vec[pol1])*measured[pol2]);
                     if (pol1 >= pol2) {
                         pxp.addModelProduct(pol1,pol2, conj(vec[pol1])*vec[pol2]);
                     }
                }
           }
           
           GenericNormalEquations ne2;
           ne2.add(cdm,pxp);
           std::vector<std::string> unknowns2 = ne2.unknowns();
           CPPUNIT_ASSERT_EQUAL(size_t(4), unknowns2.size());
           
           // check that the normal matrices obtained by these two approaches 
           // are indeed identical for all parameters (ne1 and ne2 should be identical)
           CPPUNIT_ASSERT_EQUAL(unknowns1.size(), unknowns2.size());
           for (size_t par=0; par<unknowns1.size(); ++par) {
                const std::string parName = unknowns1[par];                
                CPPUNIT_ASSERT(contained(unknowns2,parName));
                for (size_t par2=0; par2<unknowns1.size(); ++par2) {
                     const std::string parName2 = unknowns1[par2];
                     CPPUNIT_ASSERT(contained(unknowns2,parName2));
                     const casa::Matrix<double> nm1 = ne1.normalMatrix(parName,parName2);
                     const casa::Matrix<double> nm2 = ne2.normalMatrix(parName,parName2);
                     CPPUNIT_ASSERT_EQUAL(nm1.shape(),nm2.shape());
                     for (casa::uInt row=0; row<nm1.nrow(); ++row) {
                          for (casa::uInt col=0; col<nm1.ncolumn(); ++col) {
                               CPPUNIT_ASSERT_DOUBLES_EQUAL(nm1(row,col),nm2(row,col),1e-7);
                          }
                     }
                }
           }
           
           // check that data vectors obtained by these two approaches match
           
           for (size_t par=0; par<unknowns1.size(); ++par) {
                const std::string parName = unknowns1[par];                
                CPPUNIT_ASSERT(contained(unknowns2,parName));
                const casa::Vector<double> dv1 = ne1.dataVector(parName);
                const casa::Vector<double> dv2 = ne2.dataVector(parName);
                CPPUNIT_ASSERT_EQUAL(dv1.nelements(),dv2.nelements());
                for (size_t index=0; index<dv1.nelements(); ++index) {
                     // we use single precision Complex and have large numbers like 100. squared
                     // and subtracted, therefore the tolerance is set to a somewhat high value 
                     CPPUNIT_ASSERT_DOUBLES_EQUAL(dv1[index],dv2[index],1e-4);
                }
           }
        }
    protected:
        /// @brief helper method to check the presence of an element
        /// @details
        /// @param[in] cnt container
        /// @param[in] elem element
        /// @return true, if the element is present in the container
        template<typename Cont> 
        static bool contained(const Cont &cnt, const typename Cont::value_type &elem) {
           return find(cnt.begin(),cnt.end(),elem) != cnt.end();
        }
        
    };

  }
}

#endif // #ifndef GENERIC_NORMAL_EQUATION_TEST_H

