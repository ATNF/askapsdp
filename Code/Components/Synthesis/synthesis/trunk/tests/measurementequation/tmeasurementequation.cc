#include <cppunit/ui/text/TestRunner.h>

#include <ComponentEquationTest.h>
#include <VectorOperationsTest.h>
#include <ImageDFTEquationTest.h>
#include <ImageFFTEquationTest.h>
#include <GainCalibrationEquationTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::VectorOperationsTest::suite() );
  runner.addTest( conrad::synthesis::ComponentEquationTest::suite() );
  runner.addTest( conrad::synthesis::GainCalibrationEquationTest::suite() );
  //runner.addTest( conrad::synthesis::ImageDFTEquationTest::suite() );
  runner.addTest( conrad::synthesis::ImageFFTEquationTest::suite() );
  runner.run();
  return 0;
}
