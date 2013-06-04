/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2008 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///

// ASKAPsoft includes
#include <AskapTestRunner.h>

// Test includes
#include <ContinuumTests.h>
#include <ContinuumSelavyTests.h>
#include <ContinuumNVSSTests.h>
#include <ContinuumS3SEXTests.h>
#include <FullStokesContinuumTests.h>
#include <EllipseTests.h>
#include <DiscEllipseTests.h>
#include <DiscPixelTests.h>

int main(int argc, char *argv[])
{
    std::ifstream config("askap.log_cfg", std::ifstream::in);

    if (config) {
        ASKAPLOG_INIT("askap.log_cfg");
    } else {
        std::ostringstream ss;
        ss << argv[0] << ".log_cfg";
        ASKAPLOG_INIT(ss.str().c_str());
    }

    askapdev::testutils::AskapTestRunner runner(argv[0]);
    runner.addTest(askap::analysisutilities::ContinuumTest::suite());
    runner.addTest(askap::analysisutilities::ContinuumSelavyTest::suite());
    runner.addTest(askap::analysisutilities::ContinuumNVSSTest::suite());
    runner.addTest(askap::analysisutilities::ContinuumS3SEXTest::suite());
    runner.addTest(askap::analysisutilities::FullStokesContinuumTest::suite());
    runner.addTest(askap::analysisutilities::EllipseTest::suite());
    runner.addTest(askap::analysisutilities::DiscEllipseTest::suite());
    runner.addTest(askap::analysisutilities::DiscPixelTest::suite());
    bool wasSuccessful = runner.run();

    return wasSuccessful ? 0 : 1;
}
