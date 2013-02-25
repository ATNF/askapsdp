/// @file BaselineMapTest.cc
///
/// @copyright (c) 2012 CSIRO
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
#include <limits>
#include <stdint.h>
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "measures/Measures/Stokes.h"

// Classes to test
#include "simplayback/BaselineMap.h"

namespace askap {
namespace cp {

class BaselineMapTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(BaselineMapTest);
        CPPUNIT_TEST(testNormal);
        CPPUNIT_TEST(testNotFound);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            itsParset.add("baselineids", "[0..20]");
            itsParset.add("0", "[0, 0, XX]");
            itsParset.add("1", "[0, 0, XY]");
            itsParset.add("2", "[0, 0, YY]");

            itsParset.add("3", "[0, 1, XX]");
            itsParset.add("4", "[0, 1, XY]");
            itsParset.add("5", "[0, 1, YX]");
            itsParset.add("6", "[0, 1, YY]");

            itsParset.add("7", "[0, 2, XX]");
            itsParset.add("8", "[0, 2, XY]");
            itsParset.add("9", "[0, 2, YX]");
            itsParset.add("10", "[0, 2, YY]");

            itsParset.add("11", "[1, 1, XX]");
            itsParset.add("12", "[1, 1, XY]");
            itsParset.add("13", "[1, 1, YY]");

            itsParset.add("14", "[1, 2, XX]");
            itsParset.add("15", "[1, 2, XY]");
            itsParset.add("16", "[1, 2, YX]");
            itsParset.add("17", "[1, 2, YY]");

            itsParset.add("18", "[2, 2, XX]");
            itsParset.add("19", "[2, 2, XY]");
            itsParset.add("20", "[2, 2, YY]");
        };

        void tearDown() {
        }

        void testNormal() {
            BaselineMap bmap(itsParset);

            // Auto correlations
            CPPUNIT_ASSERT_EQUAL(0, bmap(0, 0, casa::Stokes::type("XX")));
            CPPUNIT_ASSERT_EQUAL(1, bmap(0, 0, casa::Stokes::type("XY")));
            CPPUNIT_ASSERT_EQUAL(2, bmap(0, 0, casa::Stokes::type("YY")));

            // Cross correlations
            CPPUNIT_ASSERT_EQUAL(3, bmap(0, 1, casa::Stokes::type("XX")));
            CPPUNIT_ASSERT_EQUAL(4, bmap(0, 1, casa::Stokes::type("XY")));
            CPPUNIT_ASSERT_EQUAL(5, bmap(0, 1, casa::Stokes::type("YX")));
            CPPUNIT_ASSERT_EQUAL(6, bmap(0, 1, casa::Stokes::type("YY")));

            // Boundry conditions
            CPPUNIT_ASSERT_EQUAL(20, bmap(2, 2, casa::Stokes::type("YY")));

            // Test all fall within range
            for (int i = 0; i <= 2; ++i) {
                for (int j = i; j <= 2; ++j) {
                    const int32_t xx = bmap(i, j, casa::Stokes::type("XX"));
                    CPPUNIT_ASSERT(xx > -1 && xx < 21);
                    const int32_t yy = bmap(i, j, casa::Stokes::type("YY"));
                    CPPUNIT_ASSERT(yy > -1 && yy < 21);
                    const int32_t xy = bmap(i, j, casa::Stokes::type("XY"));
                    CPPUNIT_ASSERT(xy > -1 && xy < 21);
                    if (i != j) {
                        const int32_t yx = bmap(i, j, casa::Stokes::type("YX"));
                        CPPUNIT_ASSERT(yx > -1 && yx < 21);
                    }
                }
            }
        }

        void testNotFound() {
            BaselineMap bmap(itsParset);
            CPPUNIT_ASSERT_EQUAL(-1, bmap(3, 3, casa::Stokes::type("XX")));
            CPPUNIT_ASSERT_EQUAL(-1, bmap(0, 0, casa::Stokes::type("I")));

            const int32_t maxuint = std::numeric_limits<int32_t>::max();
            const int32_t minuint = std::numeric_limits<int32_t>::min();
            CPPUNIT_ASSERT_EQUAL(-1, bmap(maxuint, 0, casa::Stokes::type("XX")));
            CPPUNIT_ASSERT_EQUAL(-1, bmap(minuint, 0, casa::Stokes::type("XX")));

        }

    private:
        LOFAR::ParameterSet itsParset;
};

}   // End namespace cp
}   // End namespace askap
