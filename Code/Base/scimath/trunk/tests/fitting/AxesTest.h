#include <fitting/Axes.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>
#include <vector>

namespace conrad {
namespace scimath {
	
class AxesTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(AxesTest);
    CPPUNIT_TEST(testIndices);
    CPPUNIT_TEST(testValues);
    CPPUNIT_TEST_EXCEPTION(testDuplError, std::invalid_argument);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    Axes *p1, *p2, *p3, *pempty;
    
  public:
    void setUp()
    {
      p1 = new Axes();
      p2 = new Axes();
      p3 = new Axes();
      pempty = new Axes();
    }
    
    void tearDown() 
    {
      delete p1;
      delete p2;
      delete p3;
      delete pempty;
    }
    
    void testDuplError()
    // Should throw Invalid argument
    {
      p1->add("Time", 0.0, 1.0);
      p1->add("Time", 0.0, 1.0);
    }
    
    void testCopy() 
    {
      CPPUNIT_ASSERT( !p1->has("Time"));
      p1->add("Time", 0.0, 1.0);
      CPPUNIT_ASSERT(p1->has("Time"));
      p1->add("Freq", 0.7e9, 1.7e9);
      Axes pnew(*p1);
      
      CPPUNIT_ASSERT(pnew.has("Time"));
      CPPUNIT_ASSERT(pnew.start("Time")==0.0);
      CPPUNIT_ASSERT(pnew.order("Time")==0);
      CPPUNIT_ASSERT(pnew.end("Time")==1.0);
      
      CPPUNIT_ASSERT(pnew.has("Freq"));
      CPPUNIT_ASSERT(pnew.start("Freq")==0.7e9);
      CPPUNIT_ASSERT(pnew.order("Freq")==1);
      CPPUNIT_ASSERT(pnew.end("Freq")==1.7e9);
    }
    
    void testValues()
    {
      CPPUNIT_ASSERT( !p1->has("Time"));
      p1->add("Time", 0.0, 1.0);
      CPPUNIT_ASSERT(p1->has("Time"));
      p1->add("Freq", 0.7e9, 1.7e9);
      
      CPPUNIT_ASSERT(p1->has("Time"));
      CPPUNIT_ASSERT(p1->start("Time")==0.0);
      CPPUNIT_ASSERT(p1->end("Time")==1.0);
      
      CPPUNIT_ASSERT(p1->has("Freq"));
      CPPUNIT_ASSERT(p1->start("Freq")==0.7e9);
      CPPUNIT_ASSERT(p1->end("Freq")==1.7e9);
          }  
    
    void testIndices()
    {
      CPPUNIT_ASSERT(!p1->has("Time"));
      p1->add("Time", 0.0, 1.0);
      CPPUNIT_ASSERT(p1->has("Time"));
      p1->add("Freq", 0.7e9, 1.7e9);
    }
    
  };
  
}
}
