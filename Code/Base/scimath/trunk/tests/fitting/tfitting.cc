#include <cppunit/ui/text/TestRunner.h>

#include <ParamsTest.h>
#include <ParamsTableTest.h>
#include <DesignMatrixTest.h>
#include <ImagingNormalEquationsTest.h>
#include <GenericNormalEquationsTest.h>
#include <PolynomialEquationTest.h>
#include <GeneralFittingTest.h>
#include <ComplexDiffTest.h>
#include <ComplexDiffMatrixTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( askap::scimath::ParamsTest::suite() );
  runner.addTest( askap::scimath::ParamsTableTest::suite() );
  runner.addTest( askap::scimath::DesignMatrixTest::suite() );
  runner.addTest( askap::scimath::GenericNormalEquationsTest::suite() );
  runner.addTest( askap::scimath::ImagingNormalEquationsTest::suite() );
  runner.addTest( askap::scimath::PolynomialEquationTest::suite() );
  runner.addTest( askap::scimath::GeneralFittingTest::suite() );
  runner.addTest( askap::scimath::ComplexDiffTest::suite() );
  runner.addTest( askap::scimath::ComplexDiffMatrixTest::suite() );
  runner.run();
  return 0;
}
