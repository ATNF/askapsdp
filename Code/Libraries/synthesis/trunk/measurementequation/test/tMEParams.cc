#include <measurementequation/MEParams.h>

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <vector>

using std::vector;
using std::string;

#include <iostream>

using std::ostream;
using std::cout;
using std::endl;

using namespace conrad::synthesis;

namespace conrad {
namespace synthesis {
	
class MEParamsTest : public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(MEParamsTest);
	CPPUNIT_TEST(testIndices);
	CPPUNIT_TEST(testAddition);
	CPPUNIT_TEST(testValues);
	CPPUNIT_TEST(testCongruent);
	CPPUNIT_TEST(testCompletions);
	CPPUNIT_TEST_EXCEPTION(testDuplError, casa::DuplError);
	CPPUNIT_TEST(testCopy);
	CPPUNIT_TEST_SUITE_END();
	
private:
	MEParams *p1, *p2, *p3, *pempty;
	
public:
  void setUp()
  {
    p1 = new MEParams();
    p2 = new MEParams();
    p3 = new MEParams();
    pempty = new MEParams();
  }

  void tearDown() 
  {
    delete p1;
    delete p2;
    delete p3;
    delete pempty;
  }
  
  void testDuplError()
  // Should throw DuplError
  {
	p1->add("Add0");
	p1->add("Add0");
  }

  void testCompletions() 
  {
	CPPUNIT_ASSERT( p1->size()==0);
	for (uint i=0;i<10;i++) {
  		p1->add("Root."+casa::String(i));
  		p1->add(casa::String(i)+".Root");
	}
	CPPUNIT_ASSERT(p1->regular().names().size()==20);
	CPPUNIT_ASSERT(p1->regular().completions("Roo*").size()==10);
	CPPUNIT_ASSERT(p1->regular().completions("Roo*9").size()==1);
	CPPUNIT_ASSERT(p1->regular().completions("*Root").size()==10);
	CPPUNIT_ASSERT(p1->regular().completions("*oo*").size()==20);
	CPPUNIT_ASSERT(p1->regular().completions("*2*").size()==2);
	CPPUNIT_ASSERT(p1->regular().completions("Nothing").size()==0);
  }

  void testCopy() 
  {
	CPPUNIT_ASSERT( p1->size()==0);
	p1->add("Copy0");
  	CPPUNIT_ASSERT(p1->regular().has("Copy0"));
	p1->add("Copy1", 1.5);
	CPPUNIT_ASSERT(p1);
	MEParams pnew(*p1);
	CPPUNIT_ASSERT(pnew.size()==2);
  	CPPUNIT_ASSERT(pnew.regular().has("Copy0"));
  	CPPUNIT_ASSERT(pnew.regular().has("Copy1"));
	CPPUNIT_ASSERT(pnew.regular().value("Copy1")==1.5);
  }

  void testValues()
  {
	p1->add("Value0", 1.5);
	CPPUNIT_ASSERT(p1->regular().value("Value0")==1.5);
	MEImage im("Cena.image");
	MEImage im2(im);
	p1->add("Value1", im);
  }  
  
  void testIndices()
  {
	CPPUNIT_ASSERT( p1->size()==0);
	p1->add("Ind0");
  	CPPUNIT_ASSERT(p1->regular().has("Ind0"));
	p1->add("Ind1");
  	CPPUNIT_ASSERT(!pempty->regular().has("Null"));
  }

  void testAddition()
  {
	CPPUNIT_ASSERT( p1->size()==0);
	p1->add("Add0");
	CPPUNIT_ASSERT( p1->size()==1);
	CPPUNIT_ASSERT( p1->image().size()==0);
	CPPUNIT_ASSERT( p1->regular().size()==1);
	p1->add("Add1");
	CPPUNIT_ASSERT( p1->size()==2);
	p1->update("Add1", 2.6);
	CPPUNIT_ASSERT( p1->regular().value("Add1")==2.6);
  }

  void testCongruent()
  {
	CPPUNIT_ASSERT( p1->size()==0);
	p1->add("foo");
	CPPUNIT_ASSERT( p1->size()==1);
    CPPUNIT_ASSERT( !(p1->isCongruent(*p2)));
	p2->add("bar");
	CPPUNIT_ASSERT( !(p1->isCongruent(*p2)));
	p3->add("foo");
	CPPUNIT_ASSERT( p1->isCongruent(*p3));
  }
};

}
}

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::MEParamsTest::suite() );
  runner.run();
  return 0;
}
