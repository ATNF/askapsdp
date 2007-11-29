#include <cppunit/ui/text/TestRunner.h>

#include <conrad_conrad.h>
#include <IndexedLessTest.h>
#include <ConradErrorTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::utility::IndexedLessTest::suite() );
  runner.addTest( conrad::ConradErrorTest::suite() );
  runner.run();
  return 0;
}
