#include <cppunit/ui/text/TestRunner.h>

#include <TableVisGridderTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::TableVisGridder::suite());
  runner.run();
  return 0;
}
