#include <fitting/NormalEquations.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

namespace conrad {
namespace scimath {
	
class NormalEquationsTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(NormalEquationsTest);
    CPPUNIT_TEST(testConstructors);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    NormalEquations *p1, *p2, *p3, *pempty;
    
  public:
    void setUp()
    {
      p1 = new NormalEquations();
      p2 = new NormalEquations();
      p3 = new NormalEquations();
      pempty = new NormalEquations();
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
		p1 = new NormalEquations(ip);
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
		p1 = new NormalEquations(ip);
		delete p2;
		p2 = new NormalEquations(*p1);
		CPPUNIT_ASSERT(p2->parameters().names().size()==3);
		CPPUNIT_ASSERT(p2->parameters().names()[0]=="Value0");
		CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value1");
		CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value2");
    }
    
  };
  
}
}
