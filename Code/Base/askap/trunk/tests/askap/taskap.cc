#include <cppunit/ui/text/TestRunner.h>

#include <askap_askap.h>
#include <IndexedCompareTest.h>
#include <ConradErrorTest.h>
#include <MapKeyIteratorTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::utility::IndexedLessTest::suite() );
  runner.addTest( conrad::ConradErrorTest::suite() );
  runner.addTest( conrad::utility::MapKeyIteratorTest::suite() );
  runner.run();
  return 0;
}
