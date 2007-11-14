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
      CPPUNIT_TEST(testAdd);
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

        
        void testAdd()
        {
          const casa::uInt nData = 10;
          DesignMatrix dm;
          dm.addDerivative("Value0", casa::Matrix<casa::Double>(nData, 1, 1.0));
          dm.addDerivative("Value1", casa::Matrix<casa::Double>(nData, 1, 2.0));
          dm.addResidual(casa::Vector<casa::Double>(nData, 1.0), casa::Vector<double>(nData, 1.0));
          CPPUNIT_ASSERT(dm.nData() == nData);
          itsNE->add(dm);
          CPPUNIT_ASSERT(fabs(itsNE->normalMatrix("Value0", "Value0")(0,0)-
                              double(nData))<1e-7);
          CPPUNIT_ASSERT(fabs(itsNE->normalMatrix("Value1", "Value1")(0,0)-
                              4.*nData)<1e-7);
          CPPUNIT_ASSERT(fabs(itsNE->normalMatrix("Value1", "Value0")(0,0)-
                              2.*nData)<1e-7);
          //std::cout<<itsNE->normalMatrix("Value0", "Value1")(0,0)<<std::endl;
        }

    };

  }
}

#endif // #ifndef GENERIC_NORMAL_EQUATION_TEST_H

