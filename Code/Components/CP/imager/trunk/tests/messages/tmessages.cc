/// @file tmessages.cc
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


// Test includes
#include <AllMessagesTest.h>

int main(int argc, char *argv[])
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

    // Insert test-suite and run the tests.
    CppUnit::TestRunner testrunner;
    testrunner.addTest(askap::cp::AllMessagesTest::suite());
    testrunner.run(testresult);

    // Output results in compiler-format to screen for users.
    CppUnit::CompilerOutputter compileroutputter(&collectedresults, std::cerr);
    compileroutputter.write();

    // Output XML to file for Hudson processing.
    std::string pname = argv[0];
    std::string::size_type idx = pname.find_last_of('/');

    if (idx != std::string::npos) {
        pname = pname.substr(idx+2, pname.size()); // Want just the filename.
    }

    std::stringstream filename;
    filename << "tests/" << pname << "-results.xml";
    std::ofstream outputFile(filename.str().c_str());
    CppUnit::XmlOutputter xmloutputter(&collectedresults, outputFile);
    xmloutputter.write();
    outputFile.close();

    // Return 0 if tests were successful.
    return collectedresults.wasSuccessful() ? 0 : 1;
}
