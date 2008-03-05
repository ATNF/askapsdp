#include <cppunit/ui/text/TestRunner.h>

#include <TableVisGridderTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( askap::synthesis::TableVisGridderTest::suite());
  runner.run();
  return 0;
}
