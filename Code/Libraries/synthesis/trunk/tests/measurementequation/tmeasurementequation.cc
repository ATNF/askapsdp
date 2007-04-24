#include <cppunit/ui/text/TestRunner.h>

#include <MEDomainTest.h>
#include <MEParamsTest.h>
#include <MEDesignMatrixTest.h>
#include <MENormalEquationsTest.h>
#include <MEComponentEquationTest.h>
#include <MEImageEquationTest.h>
#include <MELinearSolverTest.h>


int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::MEDomainTest::suite() );
  runner.addTest( conrad::synthesis::MEParamsTest::suite() );
  runner.addTest( conrad::synthesis::MEDesignMatrixTest::suite() );
  runner.addTest( conrad::synthesis::MENormalEquationsTest::suite() );
  runner.addTest( conrad::synthesis::MEComponentEquationTest::suite() );
  runner.addTest( conrad::synthesis::MEImageEquationTest::suite() );
  runner.addTest( conrad::synthesis::MELinearSolverTest::suite() );
  runner.run();
  return 0;
}
