#include <fitting/DesignMatrix.h>
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/ComplexDiff.h>
#include <askap/AskapError.h>

#include <cppunit/extensions/HelperMacros.h>
namespace askap
{
  namespace scimath
  {

    class DesignMatrixTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(DesignMatrixTest);
      CPPUNIT_TEST(testConstructors);
      CPPUNIT_TEST_EXCEPTION(testInvalidArgument, askap::CheckError);
      CPPUNIT_TEST(testCopy);
      CPPUNIT_TEST(testAdd);
      CPPUNIT_TEST(testComplexDiffMatrix);
      CPPUNIT_TEST_SUITE_END();

      private:
        DesignMatrix *p1, *p2, *p3, *pempty;

      public:
        void setUp()
        {
          Params ip;
          p1 = new DesignMatrix(); // (ip);
          p2 = new DesignMatrix(); //(ip);
          p3 = new DesignMatrix(); //(ip);
          pempty = new DesignMatrix(); //(ip);
        }

        void tearDown()
        {
          delete p1;
          delete p2;
          delete p3;
          delete pempty;
        }

        void testConstructors()
        {
          Params ip;
          ip.add("Value0");
          ip.add("Value1");
          ip.add("Value2");
          delete p1;
          p1 = new DesignMatrix(); //(ip);
          //CPPUNIT_ASSERT(p1->parameters().names().size()==3);
          //CPPUNIT_ASSERT(p1->parameters().names()[0]=="Value0");
          //CPPUNIT_ASSERT(p1->parameters().names()[1]=="Value1");
          //CPPUNIT_ASSERT(p1->parameters().names()[2]=="Value2");
        }

        void testCopy()
        {
          Params ip;
          ip.add("Value0");
          ip.add("Value1");
          ip.add("Value2");
          delete p1;
          p1 = new DesignMatrix(); // (ip);
          delete p2;
          p2 = new DesignMatrix(*p1);
          //CPPUNIT_ASSERT(p2->parameters().names().size()==3);
          //CPPUNIT_ASSERT(p2->parameters().names()[0]=="Value0");
          //CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value1");
          //CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value2");
        }

        void testAdd()
        {
          Params ip;
          ip.add("Value0");
          ip.add("Value1", 1.5);
          uint imsize=100;
          casa::Vector<double> im(imsize);
          im.set(3.0);
          ip.add("Image2", im);

          delete p1;
          p1 = new DesignMatrix(); //(ip);
          int gradsize=10*10*100;
          p1->addDerivative("Value0", casa::Vector<casa::Double>(100, 0.0));
          p1->addDerivative("Value1", casa::Vector<casa::Double>(100, 0.0));
          p1->addDerivative("Image2", casa::Vector<casa::Double>(gradsize, 0.0));
          p1->addResidual(casa::Vector<casa::Double>(100, 0.0), casa::Vector<double>(100, 1.0));
          CPPUNIT_ASSERT(p1->nData()==100);
          CPPUNIT_ASSERT(p1->nParameters()==3);
          
          /*
          // this is a test of the order, in which different dimensions are stored
          // by casa array. It is not used, but left here in case I forget
          // the result in the future. 
          casa::Matrix<double> mtr(2,2,0.);
          mtr(0,1)=1;
          mtr(1,0)=2;
          mtr(1,1)=3;
          for (int i=0;i<2;++i) for (int j=0;j<2;++j)
          std::cout<<i<<" "<<j<<" "<<mtr(i,j)<<std::endl;
          casa::Vector<double> vec = mtr.reform(casa::IPosition(1,4));
          for (int i=0;i<4;++i)
          std::cout<<vec[i]<<std::endl;
          */
        }
        
        void testComplexDiffMatrix()
        {
           ComplexDiffMatrix cdm(5,5, casa::Complex(0.,-1.));
           cdm(0,0) = ComplexDiff("g1", casa::Complex(110.,0.));
           cdm(3,3) = ComplexDiff("amp", 50.);
           cdm(4,3) = ComplexDiff("g2", casa::Complex(10.,-10.)) *
                        ComplexDiff("mult", casa::Complex(0.,-1.)); 
           casa::Matrix<casa::Complex> data(5,5,casa::Complex(0.,-1.));
           casa::Matrix<double> weight(5,5,1.);
           p1->addModel(cdm,data,weight);
           CPPUNIT_ASSERT(p1->nData() == 50);
           CPPUNIT_ASSERT(p1->nParameters() == 7);
        }

        void testInvalidArgument()
        {
          Params ip;
          ip.add("Value0");
// Will throw std::invalid_argument
          delete p1;
          p1 = new DesignMatrix(); //(ip);
          casa::Vector<casa::Double> mat(100, 0.0);
          p1->addDerivative("FooBar", mat);
          p1->derivative("Value0");
        }

    };

  }
}
