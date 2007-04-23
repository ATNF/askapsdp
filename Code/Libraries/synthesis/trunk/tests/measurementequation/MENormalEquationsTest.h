#include <measurementequation/MENormalEquations.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

namespace conrad {
namespace synthesis {
	
class MENormalEquationsTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(MENormalEquationsTest);
    CPPUNIT_TEST(testConstructors);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    MENormalEquations *p1, *p2, *p3, *pempty;
    
  public:
    void setUp()
    {
      p1 = new MENormalEquations();
      p2 = new MENormalEquations();
      p3 = new MENormalEquations();
      pempty = new MENormalEquations();
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
		p1 = new MENormalEquations(ip);
		CPPUNIT_ASSERT(p1->parameters().names().size()==3);
		CPPUNIT_ASSERT(p1->parameters().names()[0]=="Value0");
		CPPUNIT_ASSERT(p1->parameters().names()[1]=="Value1");
		CPPUNIT_ASSERT(p1->parameters().names()[2]=="Value2");
    }
    
    void testCopy() 
    {
		MEParams ip;
		ip.add("Value0");
		ip.add("Value1");
		ip.add("Value2");
		delete p1;
		p1 = new MENormalEquations(ip);
		delete p2;
		p2 = new MENormalEquations(*p1);
		CPPUNIT_ASSERT(p2->parameters().names().size()==3);
		CPPUNIT_ASSERT(p2->parameters().names()[0]=="Value0");
		CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value1");
		CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value2");
    }
    
  };
  
}
}
