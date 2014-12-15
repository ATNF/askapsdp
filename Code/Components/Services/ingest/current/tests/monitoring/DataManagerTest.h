/// @file DataManagerTest.cc
///
/// @copyright (c) 2014 CSIRO
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
#include "monitoring/MonitorPointStatus.h"
#include "MonitoringProvider.h" // Ice interface
#include "TypedValues.h" // Ice interface

// Classes to test
#include "monitoring/DataManager.h"

// Using
using std::string;
using std::vector;
using askap::interfaces::TypedValueIntPtr;

namespace askap {
namespace cp {
namespace ingest {

class DataManagerTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(DataManagerTest);
        CPPUNIT_TEST(testGetEmpty);
        CPPUNIT_TEST(testUpdateGet);
        CPPUNIT_TEST(testInvalidate);
        CPPUNIT_TEST(testInvalidateNonExistent);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            TEST_PREFIX = "ingest0.cp.ingest";
            TEST_POINT_NAME = "point1";
            TEST_VALUE = 1234;
            TEST_UNIT = "s";
        };

        void tearDown() {
        }

        // Test fetching an empty point set
        void testGetEmpty() {
            DataManager dm(TEST_PREFIX);
            CPPUNIT_ASSERT_EQUAL(0ul, getTestPoint(dm).size());
        }

        // Test update() and get() work correctly for the nonimal use-cases
        void testUpdateGet() {
            DataManager dm(TEST_PREFIX);
            dm.update(TEST_POINT_NAME, TEST_VALUE, MonitorPointStatus::OK, TEST_UNIT);

            vector<IceMonitorPoint> result = getTestPoint(dm);
            CPPUNIT_ASSERT_EQUAL(1ul, result.size());

            // Check the data set matches the data returned
            CPPUNIT_ASSERT_EQUAL(TEST_PREFIX + TEST_POINT_NAME, result[0].name);
            CPPUNIT_ASSERT(result[0].value->type == askap::interfaces::TypeInt);
            const TypedValueIntPtr tvptr = TypedValueIntPtr::dynamicCast(result[0].value);
            CPPUNIT_ASSERT_EQUAL(TEST_VALUE, tvptr->value);
            CPPUNIT_ASSERT(result[0].status == askap::interfaces::monitoring::OK);
            CPPUNIT_ASSERT_EQUAL(TEST_UNIT, result[0].unit);
        }

        // Tests the invalidatePoint() method for a point that does exist
        void testInvalidate() {
            DataManager dm(TEST_PREFIX);
            CPPUNIT_ASSERT_EQUAL(0ul, getTestPoint(dm).size());

            // Update (i.e. render it a valid point)
            dm.update(TEST_POINT_NAME, TEST_VALUE, MonitorPointStatus::OK, TEST_UNIT);

            // Confirm it exists
            CPPUNIT_ASSERT_EQUAL(1ul, getTestPoint(dm).size());

            // Invalidate & confirm it no longer exists
            dm.invalidatePoint(TEST_POINT_NAME);
            CPPUNIT_ASSERT_EQUAL(0ul, getTestPoint(dm).size());
        }

        // Tests the invalidatePoint() method for a point that does NOT exist
        void testInvalidateNonExistent() {
            // Just make sure no exception is thrown
            DataManager dm(TEST_PREFIX);
            dm.invalidatePoint("nonexistent_point");
        }

    private:

        // Helper method - returns the test point value
        vector<IceMonitorPoint> getTestPoint(DataManager& dm) {
            vector<string> pointnames;
            pointnames.push_back(TEST_PREFIX + TEST_POINT_NAME);
            return dm.get(pointnames);
        }

        // Some test constants (initialised in setUp())
        string TEST_PREFIX;
        string TEST_POINT_NAME;
        int TEST_VALUE;
        string TEST_UNIT;

};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
