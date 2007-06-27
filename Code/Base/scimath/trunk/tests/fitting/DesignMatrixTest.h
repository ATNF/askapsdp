#include <fitting/DesignMatrix.h>
#include <conrad/ConradError.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

namespace conrad
{
  namespace scimath
  {

    class DesignMatrixTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(DesignMatrixTest);
      CPPUNIT_TEST(testConstructors);
      CPPUNIT_TEST_EXCEPTION(testInvalidArgument, conrad::CheckError);
      CPPUNIT_TEST(testCopy);
      CPPUNIT_TEST(testAdd);
      CPPUNIT_TEST_SUITE_END();

      private:
        DesignMatrix *p1, *p2, *p3, *pempty;

      public:
        void setUp()
        {
          Params ip;
          p1 = new DesignMatrix(ip);
          p2 = new DesignMatrix(ip);
          p3 = new DesignMatrix(ip);
          pempty = new DesignMatrix(ip);
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
          p1 = new DesignMatrix(ip);
          CPPUNIT_ASSERT(p1->parameters().names().size()==3);
          CPPUNIT_ASSERT(p1->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p1->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p1->parameters().names()[2]=="Value2");
        }

        void testCopy()
        {
          Params ip;
          ip.add("Value0");
          ip.add("Value1");
          ip.add("Value2");
          delete p1;
          p1 = new DesignMatrix(ip);
          delete p2;
          p2 = new DesignMatrix(*p1);
          CPPUNIT_ASSERT(p2->parameters().names().size()==3);
          CPPUNIT_ASSERT(p2->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value2");
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
          p1 = new DesignMatrix(ip);
          int gradsize=10*10*100;
          p1->addDerivative("Value0", casa::Vector<casa::Double>(100, 0.0));
          p1->addDerivative("Value1", casa::Vector<casa::Double>(100, 0.0));
          p1->addDerivative("Image2", casa::Vector<casa::Double>(gradsize, 0.0));
          p1->addResidual(casa::Vector<casa::Double>(100, 0.0), casa::Vector<double>(100, 1.0));
          CPPUNIT_ASSERT(p1->nData()==100);
          CPPUNIT_ASSERT(p1->nParameters()==3);
        }

        void testInvalidArgument()
        {
          Params ip;
          ip.add("Value0");
// Will throw std::invalid_argument
          delete p1;
          p1 = new DesignMatrix(ip);
          casa::Vector<casa::Double> mat(100, 0.0);
          p1->addDerivative("FooBar", mat);
        }

    };

  }
}
