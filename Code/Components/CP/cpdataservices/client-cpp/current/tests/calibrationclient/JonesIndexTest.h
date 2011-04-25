/// @file JonesIndexTest.h
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
#include "casa/aipstype.h"

// Classes to test
#include "calibrationclient/JonesIndex.h"

namespace askap {
namespace cp {
namespace caldataservice {

class JonesIndexTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(JonesIndexTest);
        CPPUNIT_TEST(testGetters);
        CPPUNIT_TEST(testOperatorEqual);
        CPPUNIT_TEST(testOperatorLessThan);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testGetters() {
            const casa::Short antenna = 3;
            const casa::Short beam = 5;
            JonesIndex index(antenna, beam);

            CPPUNIT_ASSERT_EQUAL(antenna, index.getAntenna());
            CPPUNIT_ASSERT_EQUAL(beam, index.getBeam());
        }

        void testOperatorEqual() {
            const casa::Short a1 = 1;
            const casa::Short a2 = 2;
            const casa::Short b1 = 3;
            const casa::Short b2 = 4;

            JonesIndex index1(a1, b1);
            JonesIndex index2(a2, b2);
            JonesIndex index3(a1, b1);
            JonesIndex index4(a1, b2);

            CPPUNIT_ASSERT(index1 != index2);
            CPPUNIT_ASSERT(index2 != index3);
            CPPUNIT_ASSERT(index1 == index3);
            CPPUNIT_ASSERT(index1 != index4);

            CPPUNIT_ASSERT(index1 == index1);
            CPPUNIT_ASSERT(index2 == index2);
            CPPUNIT_ASSERT(index3 == index3);
            CPPUNIT_ASSERT(index4 == index4);
        }

        void testOperatorLessThan() {
            const casa::Short a1 = 1;
            const casa::Short a2 = 2;
            const casa::Short b1 = 3;
            const casa::Short b2 = 4;

            JonesIndex index1(a1, b1);
            JonesIndex index2(a2, b2);
            JonesIndex index3(a1, b1);
            JonesIndex index4(a1, b2);

            CPPUNIT_ASSERT(index1 < index2);
            CPPUNIT_ASSERT(index1 < index4);
            CPPUNIT_ASSERT(!(index2 < index4));
            CPPUNIT_ASSERT(!(index1 < index3));
            CPPUNIT_ASSERT(!(index1 < index1));
        }
};

}   // End namespace caldataservice
}   // End namespace cp
}   // End namespace askap
