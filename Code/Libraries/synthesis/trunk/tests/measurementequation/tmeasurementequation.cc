#include <cppunit/ui/text/TestRunner.h>

#include <MEDomainTest.h>
#include <MEParamsTest.h>


int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::synthesis::MEDomainTest::suite() );
  runner.addTest( conrad::synthesis::MEParamsTest::suite() );
  runner.run();
  return 0;
}
