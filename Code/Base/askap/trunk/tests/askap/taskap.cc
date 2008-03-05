#include <cppunit/ui/text/TestRunner.h>

#include <askap_askap.h>
#include <IndexedCompareTest.h>
#include <AskapErrorTest.h>
#include <MapKeyIteratorTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( askap::utility::IndexedLessTest::suite() );
  runner.addTest( askap::AskapErrorTest::suite() );
  runner.addTest( askap::utility::MapKeyIteratorTest::suite() );
  runner.run();
  return 0;
}
