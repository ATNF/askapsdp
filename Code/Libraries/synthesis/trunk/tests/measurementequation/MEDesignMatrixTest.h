#include <measurementequation/MEDesignMatrix.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

namespace conrad {
namespace synthesis {
	
class MEDesignMatrixTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(MEDesignMatrixTest);
    CPPUNIT_TEST(testConstructors);
	CPPUNIT_TEST_EXCEPTION(testInvalidArgument, std::invalid_argument);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    MEDesignMatrix *p1, *p2, *p3, *pempty;
    
  public:
    void setUp()
    {
      p1 = new MEDesignMatrix();
      p2 = new MEDesignMatrix();
      p3 = new MEDesignMatrix();
      pempty = new MEDesignMatrix();
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
		MEParams ip;
		ip.add("Value0");
		ip.add("Value1");
		ip.add("Value2");
		delete p1;
		p1 = new MEDesignMatrix(ip);
		CPPUNIT_ASSERT(p1->names().size()==3);
		CPPUNIT_ASSERT(p1->names()[0]=="Value0");
		CPPUNIT_ASSERT(p1->names()[1]=="Value1");
		CPPUNIT_ASSERT(p1->names()[2]=="Value2");
    }
    
    void testCopy() 
    {
		MEParams ip;
		ip.add("Value0");
		ip.add("Value1");
		ip.add("Value2");
		delete p1;
		p1 = new MEDesignMatrix(ip);
		delete p2;
		p2 = new MEDesignMatrix(*p1);
		CPPUNIT_ASSERT(p2->names().size()==3);
		CPPUNIT_ASSERT(p2->names()[0]=="Value0");
		CPPUNIT_ASSERT(p2->names()[1]=="Value1");
		CPPUNIT_ASSERT(p2->names()[2]=="Value2");
    }
    
    void testAdd()
    {
		MEParams ip;
		ip.add("Value0");
		ip.add("Value1", 1.5);
		uint imsize=100;
		casa::Vector<double> im(imsize);
		im.set(3.0);
		ip.add("Image2", im);

		delete p1;
		p1 = new MEDesignMatrix(ip);
		uint gradsize=10*10*100;
		p1->addDerivative("Value0", casa::Vector<casa::Complex>(100, 0.0));
		p1->addDerivative("Value1", casa::Vector<casa::Complex>(100, 0.0));
		p1->addDerivative("Image2", casa::Vector<casa::Complex>(gradsize, 0.0));
		p1->addResidual(casa::Vector<casa::Complex>(100.0, 0.0), casa::Vector<double>(100.0, 1.0));
		CPPUNIT_ASSERT(p1->derivative("Image2").nelements()==gradsize);
		CPPUNIT_ASSERT(p1->residual().nelements()==100);
    }  
    
    void testInvalidArgument()
    {
		MEParams ip;
		ip.add("Value0");
		delete p1;
		p1 = new MEDesignMatrix(ip);
		// Will throw std::invalid_argument
		p1->addDerivative("FooBar", casa::Vector<casa::Complex>(100, 0.0));
    }
    
  };
  
}
}
