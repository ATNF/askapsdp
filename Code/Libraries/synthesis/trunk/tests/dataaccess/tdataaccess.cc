/// @file tdataaccess.cc
///
/// This file runs the test suite coded in DataAccessTest/DataAccsessTestImpl
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#include <cppunit/ui/text/TestRunner.h>

#include "DataAccessTest.h"

int main(int, char **)
{
   CppUnit::TextUi::TestRunner runner;
   runner.addTest(conrad::synthesis::DataAccessTest::suite());
   runner.run();
   return 0;
}
