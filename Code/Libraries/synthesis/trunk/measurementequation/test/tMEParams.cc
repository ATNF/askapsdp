#include <measurementequation/MEParams.h>

#include <casa/Arrays/Matrix.h>

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace conrad::synthesis;

namespace conrad {
namespace synthesis {
	
class MEParamsTest : public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE( MEParamsTest );
	CPPUNIT_TEST( testCongruent );
	CPPUNIT_TEST_SUITE_END();
private:
	MEParams *p1, *p2, *p3;
public:
  void setUp()
  {
    p1 = new MEParams();
    p2 = new MEParams();
    p3 = new MEParams();
  }

  void tearDown() 
  {
    delete p1;
    delete p2;
    delete p3;
  }

  void testCongruent()
  {
    CPPUNIT_ASSERT( p1->isCongruent(*p2));
    CPPUNIT_ASSERT( !(p1->isCongruent(*p3)));
  }
};

// Someone needs these templates - I don't know who!
casa::Matrix<casa::String> c0;


}
}

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::MEParamsTest::suite() );
  runner.run();
  return 0;
}
