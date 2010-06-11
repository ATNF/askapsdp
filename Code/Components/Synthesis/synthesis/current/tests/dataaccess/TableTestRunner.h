/// @file
/// @brief TestRunner for tests working with a table (MS)
/// @details This file contains a class derived from CppUnit's TestRunner
/// It adds the following functionality:
/// @li in the constructor the test measurement set is copied to
///     the local directory and named "./.test.ms"
/// @li in the destructor this scratch measurement set is removed
/// @li there is a method to obtain the name of the measurement set
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#ifndef __TABLE_TEST_RUNNER_H
#define __TABLE_TEST_RUNNER_H

// cppunit includes
#include <AskapTestRunner.h>

// std includes
#include <string>

// askap includes
#include <askap/AskapError.h>

// casa includes
#include <tables/Tables/Table.h>
#include <tables/Tables/TableError.h>
#include <casa/OS/EnvVar.h>


namespace askap {

namespace synthesis {

/// @brief TestRunner for tests working with a table (MS)
/// @details This class is derived from CppUnit's TestRunner
/// It adds the following functionality:
/// @li in the constructor the test measurement set is copied to
///     the local directory and named "./.test.ms"
/// @li in the destructor this scratch measurement set is removed
struct TableTestRunner : public askapdev::testutils::AskapTestRunner
{
  /// @brief copy the table from the location within the ASKAP tree
  TableTestRunner(const std::string& name);
  
  /// @brief delete the scratch table
  ~TableTestRunner();
  
  /// @return a name of the test dataset (scratch table to work with)
  static const std::string& msName() throw() {return theirTestMSName;}
private:
  static std::string theirTestMSName;
};

TableTestRunner::TableTestRunner(const std::string& name)
    : AskapTestRunner(name) {
  if (theirTestMSName!="") {
      throw AskapError("There supposed to be only one instance of TableTestRunner");
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
      ASKAPTHROW(AskapError, "Problems in making a copy of the test measurement set. "<<
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
      ASKAPTHROW(AskapError, "Problems deleting the scratch table");
  }
}

std::string TableTestRunner::theirTestMSName;

} // namespace synthesis

} // namespace askap

#endif // #ifndef __TABLE_TEST_RUNNER_H
