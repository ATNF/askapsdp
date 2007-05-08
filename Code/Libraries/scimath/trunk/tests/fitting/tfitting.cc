#include <cppunit/ui/text/TestRunner.h>

#include <DomainTest.h>
#include <ParamsTest.h>
#include <DesignMatrixTest.h>
#include <NormalEquationsTest.h>


int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( conrad::scimath::DomainTest::suite() );
  runner.addTest( conrad::scimath::ParamsTest::suite() );
  runner.addTest( conrad::scimath::DesignMatrixTest::suite() );
  runner.addTest( conrad::scimath::NormalEquationsTest::suite() );
  runner.run();
  return 0;
}
