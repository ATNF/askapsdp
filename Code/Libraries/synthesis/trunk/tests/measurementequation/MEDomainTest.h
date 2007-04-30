#include <measurementequation/MEDomain.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>
#include <vector>

namespace conrad {
namespace synthesis {
	
class MEDomainTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(MEDomainTest);
    CPPUNIT_TEST(testIndices);
    CPPUNIT_TEST(testValues);
    CPPUNIT_TEST_EXCEPTION(testDuplError, std::invalid_argument);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    MEDomain *p1, *p2, *p3, *pempty;
    
  public:
    void setUp()
    {
      p1 = new MEDomain();
      p2 = new MEDomain();
      p3 = new MEDomain();
      pempty = new MEDomain();
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
      p1->add("Time", 0.0, 1.0, 128);
      p1->add("Time", 0.0, 1.0, 128);
    }
    
    void testCopy() 
    {
      CPPUNIT_ASSERT( !p1->has("Time"));
      p1->add("Time", 0.0, 1.0, 128);
      CPPUNIT_ASSERT(p1->has("Time"));
      p1->add("Freq", 0.7e9, 1.7e9, 16384);
      MEDomain pnew(*p1);
      std::vector<int> shape(p1->shape());
      CPPUNIT_ASSERT(shape[0]==128);
      CPPUNIT_ASSERT(shape[1]==16384);
      
      CPPUNIT_ASSERT(pnew.has("Time"));
      CPPUNIT_ASSERT(pnew.start("Time")==0.0);
      CPPUNIT_ASSERT(pnew.order("Time")==0);
      CPPUNIT_ASSERT(pnew.end("Time")==1.0);
      CPPUNIT_ASSERT(pnew.cells("Time")==128);
      
      CPPUNIT_ASSERT(pnew.has("Freq"));
      CPPUNIT_ASSERT(pnew.start("Freq")==0.7e9);
      CPPUNIT_ASSERT(pnew.order("Freq")==1);
      CPPUNIT_ASSERT(pnew.end("Freq")==1.7e9);
      CPPUNIT_ASSERT(pnew.cells("Freq")==16384);
    }
    
    void testValues()
    {
      CPPUNIT_ASSERT( !p1->has("Time"));
      p1->add("Time", 0.0, 1.0, 128);
      CPPUNIT_ASSERT(p1->has("Time"));
      p1->add("Freq", 0.7e9, 1.7e9, 16384);
      
      CPPUNIT_ASSERT(p1->has("Time"));
      CPPUNIT_ASSERT(p1->start("Time")==0.0);
      CPPUNIT_ASSERT(p1->end("Time")==1.0);
      CPPUNIT_ASSERT(p1->cells("Time")==128);

      CPPUNIT_ASSERT(p1->has("Freq"));
      CPPUNIT_ASSERT(p1->start("Freq")==0.7e9);
      CPPUNIT_ASSERT(p1->end("Freq")==1.7e9);
      CPPUNIT_ASSERT(p1->cells("Freq")==16384);
    }  
    
    void testIndices()
    {
      CPPUNIT_ASSERT(!p1->has("Time"));
      p1->add("Time", 0.0, 1.0, 128);
      CPPUNIT_ASSERT(p1->has("Time"));
      p1->add("Freq", 0.7e9, 1.7e9, 16384);
    }
    
  };
  
}
}
