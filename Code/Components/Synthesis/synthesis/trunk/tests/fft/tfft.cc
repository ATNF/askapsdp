#include <cppunit/ui/text/TestRunner.h>

#include <FFTTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( askap::synthesis::FFTTest::suite());
  runner.run();
  return 0;
}
