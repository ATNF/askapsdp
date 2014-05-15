/// @file ParsetUtilsTest.cc
///
/// @copyright (c) 2013 CSIRO
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
#include "Common/ParameterSet.h"

// Classes to test
#include "mssplit/ParsetUtils.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

class ParsetUtilsTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(ParsetUtilsTest);
        CPPUNIT_TEST(testIsInteger);
        CPPUNIT_TEST(testIsRange);
        CPPUNIT_TEST(testParseIntRange);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testIsInteger() {
            CPPUNIT_ASSERT(ParsetUtils::isInteger("300"));

            CPPUNIT_ASSERT(!ParsetUtils::isInteger("[300]"));
            CPPUNIT_ASSERT(!ParsetUtils::isInteger("[1, 2, 3]"));
            CPPUNIT_ASSERT(!ParsetUtils::isInteger("1-300"));
            CPPUNIT_ASSERT(!ParsetUtils::isInteger("1 - 300"));
        }

        void testIsRange() {
            CPPUNIT_ASSERT(ParsetUtils::isRange("1-300"));
            CPPUNIT_ASSERT(ParsetUtils::isRange("1 - 300"));

            CPPUNIT_ASSERT(!ParsetUtils::isRange("1"));
            CPPUNIT_ASSERT(!ParsetUtils::isRange("[1, 2]"));
        }

        void testParseIntRange() {
            LOFAR::ParameterSet parset;
            parset.add("mykey", "1 - 300");
            std::pair<unsigned int, unsigned int> p = ParsetUtils::parseIntRange(parset, "mykey");
            CPPUNIT_ASSERT_EQUAL(1u, p.first);
            CPPUNIT_ASSERT_EQUAL(300u, p.second);
        }

};

}   // End namespace pipelinetasks
}   // End namespace cp
}   // End namespace askap
