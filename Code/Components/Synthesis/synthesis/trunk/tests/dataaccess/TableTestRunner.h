/// @file
/// @brief TestRunner for tests working with a table (MS)
/// @details This file contains a class derived from CppUnit's TestRunner
/// It adds the following functionality:
/// @li in the constructor the test measurement set is copied to
///     the local directory and named "./.test.ms"
/// @li in the destructor this scratch measurement set is removed
/// @li there is a method to obtain the name of the measurement set
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#ifndef __TABLE_TEST_RUNNER_H
#define __TABLE_TEST_RUNNER_H

// cppunit includes
#include <cppunit/ui/text/TestRunner.h>

// std includes
#include <string>

// conrad includes
#include <conrad/ConradError.h>

// casa includes
#include <tables/Tables/Table.h>
#include <tables/Tables/TableError.h>
#include <casa/OS/EnvVar.h>


namespace conrad {

namespace synthesis {

/// @brief TestRunner for tests working with a table (MS)
/// @details This class is derived from CppUnit's TestRunner
/// It adds the following functionality:
/// @li in the constructor the test measurement set is copied to
///     the local directory and named "./.test.ms"
/// @li in the destructor this scratch measurement set is removed
struct TableTestRunner : public CppUnit::TextUi::TestRunner
{
  /// @brief copy the table from the location within the CONRAD tree
  TableTestRunner();
  
  /// @brief delete the scratch table
  ~TableTestRunner();
  
  /// @return a name of the test dataset (scratch table to work with)
  static const std::string& msName() throw() {return theirTestMSName;}
private:
  static std::string theirTestMSName;
};

TableTestRunner::TableTestRunner() {
  if (theirTestMSName!="") {
      throw ConradError("There supposed to be only one instance of TableTestRunner");
  }
  theirTestMSName="./.test.ms";

  std::string path2TestMS="../../testdata/trunk/testdataset.ms";
  if (casa::EnvironmentVariable::isDefined("ASKAP_ROOT")) {
      path2TestMS=casa::EnvironmentVariable::get("ASKAP_ROOT")+
                "/Code/Components/Synthesis/testdata/trunk/testdataset.ms";
  }
  try {
    casa::Table originalMS(path2TestMS);
    originalMS.deepCopy(theirTestMSName,casa::Table::New);
  }
  catch (const casa::AipsError &ae) {
      CONRADTHROW(ConradError, "Problems in making a copy of the test measurement set. "<<
                  "Either the current directory is not writable, or the test measurement set "<<
                  "doesn't exist. AipsError: "<<ae.what());
  }  
}

/// @brief delete the scratch table
TableTestRunner::~TableTestRunner()
{
  try {
    casa::Table copiedMS(theirTestMSName,casa::Table::Update);
    copiedMS.markForDelete();
  }
  catch (const casa::AipsError &ae) {
      CONRADTHROW(ConradError, "Problems deleting the scratch table");
  }
}

std::string TableTestRunner::theirTestMSName;

} // namespace synthesis

} // namespace conrad

#endif // #ifndef __TABLE_TEST_RUNNER_H
