/// @file ParsetUtilsTest.cc
///
/// @copyright (c) 2011 CSIRO
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

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include <string>
#include <vector>
#include <cmath>
#include "askap/AskapError.h"
#include "measures/Measures/MDirection.h"
#include "casa/Quanta/Quantum.h"

// Classes to test
#include "cmodel/ParsetUtils.h"

using namespace casa;

namespace askap {
namespace cp {
namespace pipelinetasks {

class ParsetUtilsTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(ParsetUtilsTest);
        CPPUNIT_TEST(testAsMDirection);
        CPPUNIT_TEST(testAsQuantity);
        CPPUNIT_TEST(testAsQuantityException);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testAsMDirection() {
            std::vector<std::string> input;
            input.push_back("12h30m00.00");
            input.push_back("-45.00.00.00");
            input.push_back("J2000");

            const casa::MDirection dir = ParsetUtils::asMDirection(input);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-172.5, dir.getAngle("deg").getValue()(0),
                    dblTolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-45.0, dir.getAngle("deg").getValue()(1),
                    dblTolerance);
        }

        void testAsQuantity() {
            casa::Quantum<casa::Double> output = ParsetUtils::asQuantity("2.5arcsec", "arcsec");
            CPPUNIT_ASSERT(output.isConform("arcsec"));
            CPPUNIT_ASSERT(output.isConform("arcmin"));
            CPPUNIT_ASSERT(output.isConform("deg"));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2.5, output.getValue(), dblTolerance);

            output = ParsetUtils::asQuantity("180 deg", "deg");
            CPPUNIT_ASSERT(output.isConform("arcsec"));
            CPPUNIT_ASSERT(output.isConform("arcmin"));
            CPPUNIT_ASSERT(output.isConform("deg"));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(180.0, output.getValue(), dblTolerance);

            output = ParsetUtils::asQuantity("12h30m00.00", "deg");
            CPPUNIT_ASSERT(output.isConform("arcsec"));
            CPPUNIT_ASSERT(output.isConform("arcmin"));
            CPPUNIT_ASSERT(output.isConform("deg"));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(187.5, output.getValue(), dblTolerance);

            output = ParsetUtils::asQuantity("-45.00.00.00", "deg");
            CPPUNIT_ASSERT(output.isConform("arcsec"));
            CPPUNIT_ASSERT(output.isConform("arcmin"));
            CPPUNIT_ASSERT(output.isConform("deg"));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-45.0, output.getValue(), dblTolerance);

            output = ParsetUtils::asQuantity("1.420GHz", "GHz");
            CPPUNIT_ASSERT(output.isConform("Hz"));
            CPPUNIT_ASSERT(output.isConform("MHz"));
            CPPUNIT_ASSERT(output.isConform("GHz"));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.420, output.getValue(), dblTolerance);

            output = ParsetUtils::asQuantity("304MHz", "MHz");
            CPPUNIT_ASSERT(output.isConform("Hz"));
            CPPUNIT_ASSERT(output.isConform("MHz"));
            CPPUNIT_ASSERT(output.isConform("GHz"));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(304, output.getValue(), dblTolerance);

            output = ParsetUtils::asQuantity("1mJy", "mJy");
            CPPUNIT_ASSERT(output.isConform("Jy"));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1, output.getValue(), dblTolerance);
        }

        void testAsQuantityException() {
            CPPUNIT_ASSERT_THROW(ParsetUtils::asQuantity("2.5GHz", "arcsec"), askap::AskapError);
            CPPUNIT_ASSERT_THROW(ParsetUtils::asQuantity("180deg", "Hz"), askap::AskapError);
            CPPUNIT_ASSERT_THROW(ParsetUtils::asQuantity("180deg", "Jy"), askap::AskapError);
            CPPUNIT_ASSERT_THROW(ParsetUtils::asQuantity("1mJy", "Hz"), askap::AskapError);
        }

        private:
            static const double dblTolerance = 10e-14;
};

}   // End namespace pipelinetasks
}   // End namespace cp
}   // End namespace askap
