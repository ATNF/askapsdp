#include <cppunit/ui/text/TestRunner.h>

#include <ComponentEquationTest.h>
#include <ImageDFTEquationTest.h>
#include <ImageFFTEquationTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::ComponentEquationTest::suite() );
  runner.addTest( conrad::synthesis::ImageDFTEquationTest::suite() );
  runner.addTest( conrad::synthesis::ImageFFTEquationTest::suite() );
  runner.run();
  return 0;
}
