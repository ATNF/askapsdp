/// @file
///
/// @brief Tests of the functionality provided by VectorOperations
/// @details This file contains appropriate unit tests
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au
///



// own includes
#include <measurementequation/VectorOperations.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <casa/BasicSL/Complex.h>
#include <cppunit/extensions/HelperMacros.h>
#include <conrad/ConradError.h>
#include <fitting/ComplexDiff.h>


// std includes
#include <cmath>
#include <vector>

#ifndef VECTOR_OPERATIONS_TEST_H
#define VECTOR_OPERATIONS_TEST_H

using std::abs;

#include <boost/shared_ptr.hpp>

using namespace conrad;
using namespace conrad::scimath;

namespace conrad
{
  namespace synthesis
  {

    class VectorOperationsTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(VectorOperationsTest);
      CPPUNIT_TEST(testCopy);
      CPPUNIT_TEST(testSubtract);
      CPPUNIT_TEST(testAdd);
      CPPUNIT_TEST(test1); // temporary
      CPPUNIT_TEST_SUITE_END();

      
    public:
        void testCopy()
        {
          casa::Matrix<casa::Double> in(2,2,1.);
          vector<double> vec(2,3.);
          vec[0]=-3.;
          copyVector(vec,in.row(0));
          CPPUNIT_ASSERT(fabs(in(0,0)+3.)<1e-10 && fabs(in(0,1)-3.)<1e-10);
          casa::Vector<casa::Complex> complexVec(1,casa::Complex(-1.,-2.));
          copyVector(complexVec,in.row(1));
          CPPUNIT_ASSERT(fabs(in(1,0)+1.)<1e-10 && fabs(in(1,1)+2.)<1e-10);
          vector<casa::AutoDiff<double> > autoDiffVec(2, 
                                          casa::AutoDiff<double>(0.,1));
          autoDiffVec[0]=sin(casa::AutoDiff<double>(0.,1,0));
          autoDiffVec[1]=1.+cos(casa::AutoDiff<double>(casa::C::_2pi/4.,1,0));
          copyVector(autoDiffVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0])<1e-10 && fabs(vec[1]-1)<1e-10);
          copyDerivativeVector(0,autoDiffVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0]-1)<1e-10 && fabs(vec[1]+1)<1e-10);
          vector<scimath::ComplexDiff> complexDiffVec(1,
                        scimath::ComplexDiff("par1",casa::Complex(0.,-1.)));
          complexDiffVec[0] *= scimath::ComplexDiff("par2", casa::Complex(2.,0.));
          copyVector(complexDiffVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0])<1e-10 && fabs(vec[1]+2.)<1e-10);
          copyReDerivativeVector("par1",complexDiffVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0]-2)<1e-10 && fabs(vec[1])<1e-10);
          copyReDerivativeVector("par2",complexDiffVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0])<1e-10 && fabs(vec[1]+1)<1e-10);
          copyImDerivativeVector("par1",complexDiffVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0])<1e-10 && fabs(vec[1]-2)<1e-10);
          copyImDerivativeVector("par2",complexDiffVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0]-1)<1e-10 && fabs(vec[1])<1e-10);
        }
        
        void testSubtract()
        {
          casa::Matrix<casa::Double> in(2,2,1.);
          vector<double> vec(2,3.);
          vec[0]=-3.;
          subtractVector(vec,in.row(1));
          CPPUNIT_ASSERT(fabs(in(0,0)-1.)<1e-10 && fabs(in(0,1)-1.)<1e-10);
          CPPUNIT_ASSERT(fabs(in(1,0)-4.)<1e-10 && fabs(in(1,1)+2.)<1e-10);
          casa::Vector<casa::Complex> complexVec(1,casa::Complex(-1.,-2.));
          subtractVector(complexVec,in.row(1));
          CPPUNIT_ASSERT(fabs(in(1,0)-5.)<1e-10 && fabs(in(1,1))<1e-10);
          vector<casa::AutoDiff<double> > autoDiffVec(2, 
                                          casa::AutoDiff<double>(0.,1));
          autoDiffVec[0]=sin(casa::AutoDiff<double>(0.,1,0));
          autoDiffVec[1]=1.+cos(casa::AutoDiff<double>(casa::C::_2pi/4.,1,0));
          subtractVector(autoDiffVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0]+3.)<1e-10 && fabs(vec[1]-2.)<1e-10);
          subtractVector(complexVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0]+2.)<1e-10 && fabs(vec[1]-4.)<1e-10);
        }
        
        void testAdd()
        {
          casa::Matrix<casa::Double> in(2,2,1.);
          vector<double> vec(2,3.);
          vec[0]=-3.;
          addVector(vec,in.row(1));
          CPPUNIT_ASSERT(fabs(in(0,0)-1.)<1e-10 && fabs(in(0,1)-1.)<1e-10);
          CPPUNIT_ASSERT(fabs(in(1,0)+2.)<1e-10 && fabs(in(1,1)-4.)<1e-10);
          casa::Vector<casa::Complex> complexVec(1,casa::Complex(-1.,-2.));
          addVector(complexVec,in.row(1));
          CPPUNIT_ASSERT(fabs(in(1,0)+3.)<1e-10 && fabs(in(1,1)-2.)<1e-10);
          vector<casa::AutoDiff<double> > autoDiffVec(2, 
                                          casa::AutoDiff<double>(0.,1));
          autoDiffVec[0]=sin(casa::AutoDiff<double>(0.,1,0));
          autoDiffVec[1]=1.+cos(casa::AutoDiff<double>(casa::C::_2pi/4.,1,0));
          addVector(autoDiffVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0]+3.)<1e-10 && fabs(vec[1]-4.)<1e-10);
          addVector(complexVec,vec);
          CPPUNIT_ASSERT(fabs(vec[0]+4.)<1e-10 && fabs(vec[1]-2.)<1e-10);
        }
         
        // temporary method for autodiff experiments
        void test1()
        {
          vector<casa::AutoDiff<casa::Complex> > autoDiffVec(2, 
                                          casa::AutoDiff<casa::Complex>(casa::Complex(0.,0.),1));
          autoDiffVec[0]=sin(casa::AutoDiff<casa::Complex>(0.,1,0))+casa::Complex(0,1.)*cos(casa::AutoDiff<casa::Complex>(0.,1,0));
          //autoDiffVec[1]=casa::Complex(1.)+cos(casa::AutoDiff<casa::Complex>(casa::C::_2pi/4.,1,0));
          autoDiffVec[1]=exp(casa::AutoDiff<casa::Complex>(casa::C::_2pi/4.,1,0)*casa::Complex(0,1.));
          //std::cout<<autoDiffVec[0].value()<<" "<<autoDiffVec[0].derivative(0)<<std::endl;
          //std::cout<<autoDiffVec[1].value()<<" "<<autoDiffVec[1].derivative(0)<<std::endl;
          
        }

    };

  } // namespace synthesis
} // namespace conrad

#endif // #ifndef VECTOR_OPERATIONS_TEST_H
