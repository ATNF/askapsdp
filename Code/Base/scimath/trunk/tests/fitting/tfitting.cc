#include <cppunit/ui/text/TestRunner.h>

#include <ParamsTest.h>
#include <ParamsTableTest.h>
#include <DesignMatrixTest.h>
#include <NormalEquationsTest.h>
#include <GenericNormalEquationsTest.h>
#include <PolynomialEquationTest.h>
#include <GeneralFittingTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::scimath::ParamsTest::suite() );
  runner.addTest( conrad::scimath::ParamsTableTest::suite() );
  runner.addTest( conrad::scimath::DesignMatrixTest::suite() );
  runner.addTest( conrad::scimath::GenericNormalEquationsTest::suite() );
  runner.addTest( conrad::scimath::NormalEquationsTest::suite() );
  runner.addTest( conrad::scimath::PolynomialEquationTest::suite() );
  runner.addTest( conrad::scimath::GeneralFittingTest::suite() );
  runner.run();
  return 0;
}
