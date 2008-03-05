#include <cppunit/ui/text/TestRunner.h>

#include <ComponentEquationTest.h>
#include <VectorOperationsTest.h>
#include <ImageDFTEquationTest.h>
#include <ImageFFTEquationTest.h>
#include <CalibrationMETest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( askap::synthesis::VectorOperationsTest::suite() );
  runner.addTest( askap::synthesis::ComponentEquationTest::suite() );
  runner.addTest( askap::synthesis::CalibrationMETest::suite() );
  //runner.addTest( askap::synthesis::ImageDFTEquationTest::suite() );
  runner.addTest( askap::synthesis::ImageFFTEquationTest::suite() );
  runner.run();
  return 0;
}
