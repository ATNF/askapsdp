#include <cppunit/ui/text/TestRunner.h>

#include <IndexedLessTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::utility::IndexedLessTest::suite() );
  runner.run();
  return 0;
}
