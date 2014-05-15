/// @file AskapTestRunner.cc
///
/// @copyright (c) 2009 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "AskapTestRunner.h"

// System includes
#include <string>
#include <fstream>
#include <sstream>

// CPPUnit includes
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/Test.h>

using namespace askapdev::testutils;

AskapTestRunner::AskapTestRunner(const std::string& testname)
    : itsTestname(testname)
{
}

AskapTestRunner::~AskapTestRunner()
{
}

void AskapTestRunner::addTest(CppUnit::Test *test)
{
    itsRunner.addTest(test);
}

bool AskapTestRunner::run(void)
{
    // Informs test-listener about testresults.
    CppUnit::TestResult testresult;

    // Register listener for collecting the test-results.
    CppUnit::TestResultCollector collectedresults;
    testresult.addListener(&collectedresults);

    // Register listener for per-test progress output.
    // The TextTestProgressListener will produce the '...F...' style output.
    CppUnit::TextTestProgressListener progress;
    testresult.addListener(&progress);

    // Run the tests.
    itsRunner.run(testresult);

    // Output results in compiler-format to screen for users.
    CppUnit::CompilerOutputter compileroutputter(&collectedresults, std::cerr);
    compileroutputter.write();

    // Generate a consistent XML output filename based on the program name
    // but without the leading 't'.  The '+2' on the index is to account for
    // leading '/' and 't'.
    std::string name = itsTestname;
    std::string::size_type idx = name.find_last_of('/');
    if (idx != std::string::npos) {
        name = name.substr(idx+2, name.size());
    }
    std::stringstream filename;
    filename << "tests/" << name << "-cppunit-results.xml";

    // Output XML to file for Hudson processing.
    std::ofstream outputFile(filename.str().c_str());
    CppUnit::XmlOutputter xmloutputter(&collectedresults, outputFile);
    xmloutputter.write();
    outputFile.close();

    return collectedresults.wasSuccessful();
}
