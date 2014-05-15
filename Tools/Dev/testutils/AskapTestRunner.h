/// @file AskapTestRunner.h
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

#ifndef ASKAP_TOOLS_ASKAPTESTRUNNER_H
#define ASKAP_TOOLS_ASKAPTESTRUNNER_H

// CppUnit includes
#include <cppunit/Test.h>
#include <cppunit/ui/text/TestRunner.h>

namespace askapdev {
namespace testutils {

    /// @brief This class provides a simple wrapper around
    ///        CppUnit::TextUi::TestRunner.
    ///
    /// @detail his class provides a simple wrapper around
    /// CppUnit::TextUi::TestRunner and ensured as well as the usual output
    /// which is sent to stdout, an XmlOutputter is used to produce XML
    /// output which is then placed in the tests directory.
    ///
    /// This class would then be used like so:
    ///
    /// @code
    /// int main(int argc, char *argv[])
    /// {
    ///     askapdev::testutils::AskapTestRunner runner(argv[0]);
    ///     runner.addTest(askap::MyTests::suite());
    ///     bool wasSucessful = runner.run();
    ///
    ///     return wasSucessful ? 0 : 1;
    /// }
    /// @endcode
    ///
    /// Passing argv[0] to the constructor allows the output file to be named
    /// appropriatly. So if the executable name is "mytest" the output file
    /// will be named mytest-cppunit-results.xml and will be placed in the
    /// ./tests directory.
    ///
    class AskapTestRunner {
        public:

            /// @brief Constructor.
            ///
            /// @param[in]  testname    the name of the test, which is used for
            ///     naming of the output file. This would usually be the test
            ///     name and hence this class simply allows the contents of
            ///     argv[0] be passed. If this string contains the full path
            ///     then this will be stripped off.
            AskapTestRunner(const std::string& testname);

            /// @brief Destructor.
            virtual ~AskapTestRunner();

            /// @brief Add a CppUnit test to this runner.
            ///
            /// @param[in]  test    a CppUnit test to add.
            virtual void addTest(CppUnit::Test *test);

            /// @brief Run all added tests.
            ///
            /// @return Returns whether the entire test was successful or not,
            //          true for pass, false for fail.
            virtual bool run(void);

        private:
            CppUnit::TextUi::TestRunner itsRunner;
            const std::string itsTestname;
    };
};
};

#endif
