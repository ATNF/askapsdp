#include <cppunit/ui/text/TestRunner.h>

#include <MEDomainTest.h>
#include <MEParamsTest.h>
#include <MEDesignMatrixTest.h>
#include <MENormalEquationsTest.h>
#include <MEComponentEquationTest.h>


int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::MEDomainTest::suite() );
  runner.addTest( conrad::synthesis::MEParamsTest::suite() );
  runner.addTest( conrad::synthesis::MEDesignMatrixTest::suite() );
  runner.addTest( conrad::synthesis::MENormalEquationsTest::suite() );
  runner.addTest( conrad::synthesis::MEComponentEquationTest::suite() );
  runner.run();
  return 0;
}
