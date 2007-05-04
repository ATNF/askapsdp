#include <cppunit/ui/text/TestRunner.h>

#include <ComponentEquationTest.h>
#include <ImageEquationTest.h>


int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::ComponentEquationTest::suite() );
  runner.addTest( conrad::synthesis::ImageEquationTest::suite() );
  runner.run();
  return 0;
}
