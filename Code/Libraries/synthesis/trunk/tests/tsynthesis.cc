#include <cppunit/ui/text/TestRunner.h>

#include <measurementequation/ComponentEquationTest.h>
#include <measurementequation/ImageEquationTest.h>

#include <gridding/TableVisGridderTest.h>


//#include <LinearSolverTest.h>


int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  
  runner.addTest( conrad::synthesis::TableVisGridderTest::suite());

  runner.addTest( conrad::synthesis::ComponentEquationTest::suite() );
//  runner.addTest( conrad::synthesis::ImageEquationTest::suite() );

  runner.run();
  return 0;
}
