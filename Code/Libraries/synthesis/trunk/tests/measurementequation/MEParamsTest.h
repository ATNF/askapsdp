#include <measurementequation/MEParams.h>

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>

#include <cppunit/extensions/HelperMacros.h>

namespace conrad {
namespace synthesis {
	
 class MEParamsTest : public CppUnit::TestFixture  {
    
    CPPUNIT_TEST_SUITE(MEParamsTest);
    CPPUNIT_TEST(testIndices);
    CPPUNIT_TEST(testAddition);
    //	CPPUNIT_TEST(testValues);
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
      	casa::String name;
		{
      		std::ostringstream s;
      		s<<"Root." << i;
			p1->add(s.str());
		}
		{
      		std::ostringstream s;
	      	s<<i<<".Root";
			p1->add(s.str());
		}
      }
      CPPUNIT_ASSERT(p1->names().size()==20);
      CPPUNIT_ASSERT(p1->completions("Roo*9").size()==1);
      CPPUNIT_ASSERT(p1->completions("Root.*").size()==10);
      CPPUNIT_ASSERT(p1->completions("*Root").size()==10);
      CPPUNIT_ASSERT(p1->completions("*oo*").size()==20);
      CPPUNIT_ASSERT(p1->completions("*2*").size()==2);
      CPPUNIT_ASSERT(p1->completions("Nothing").size()==0);
    }
    
    void testCopy() 
    {
      CPPUNIT_ASSERT( p1->size()==0);
      p1->add("Copy0");
      CPPUNIT_ASSERT(p1->has("Copy0"));
      p1->add("Copy1");
      CPPUNIT_ASSERT(p1);
      MEParams pnew(*p1);
      CPPUNIT_ASSERT(pnew.size()==2);
      CPPUNIT_ASSERT(pnew.has("Copy0"));
      CPPUNIT_ASSERT(pnew.has("Copy1"));
      // CPPUNIT_ASSERT(pnew.value("Copy1")==1.5);
    }
    
    //  void testValues()
    //  {
    //	p1->add("Value0", 1.5);
    //	CPPUNIT_ASSERT(p1->value("Value0")==1.5);
    //	MEImage im("Cena.image");
    //	MEImage im2(im);
    //	p1->add("Value1", im);
    //  }  
    //  
    void testIndices()
    {
      CPPUNIT_ASSERT( p1->size()==0);
      p1->add("Ind0");
      CPPUNIT_ASSERT(p1->has("Ind0"));
      p1->add("Ind1");
      CPPUNIT_ASSERT(!pempty->has("Null"));
    }
    
    void testAddition()
    {
      CPPUNIT_ASSERT( p1->size()==0);
      p1->add("Add0");
      CPPUNIT_ASSERT( p1->size()==1);
      p1->add("Add1");
      CPPUNIT_ASSERT( p1->size()==2);
      //	p1->update("Add1", 2.6);
      //	CPPUNIT_ASSERT( p1->value("Add1")==2.6);
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
