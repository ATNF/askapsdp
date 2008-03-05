/// @file tdataaccess.cc
///
/// This file runs the test suite coded in DataAccessTest/DataAccsessTestImpl
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#include <cppunit/ui/text/TestRunner.h>
#include <iostream>

#include "DataAccessTest.h"
#include "DataConverterTest.h"
#include "TableDataAccessTest.h"

#include "TableTestRunner.h"

int main(int, char **)
{
 try {
   //CppUnit::TextUi::TestRunner runner;
   askap::synthesis::TableTestRunner runner;
   runner.addTest(askap::synthesis::DataConverterTest::suite());
   runner.addTest(askap::synthesis::DataAccessTest::suite());
   runner.addTest(askap::synthesis::TableDataAccessTest::suite());
   runner.run();
   return 0;
 }
 catch (const askap::AskapError &ce) {
	 std::cerr<<ce.what()<<std::endl;
 }
}
