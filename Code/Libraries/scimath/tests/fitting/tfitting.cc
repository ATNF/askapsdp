#include <cppunit/ui/text/TestRunner.h>

#include <DomainTest.h>
#include <ParamsTest.h>
#include <DesignMatrixTest.h>
#include <NormalEquationsTest.h>
//#include <LinearSolverTest.h>


int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::DomainTest::suite() );
  runner.addTest( conrad::synthesis::ParamsTest::suite() );
  runner.addTest( conrad::synthesis::DesignMatrixTest::suite() );
  runner.addTest( conrad::synthesis::NormalEquationsTest::suite() );
//  runner.addTest( conrad::synthesis::LinearSolverTest::suite() );
  runner.run();
  return 0;
}
