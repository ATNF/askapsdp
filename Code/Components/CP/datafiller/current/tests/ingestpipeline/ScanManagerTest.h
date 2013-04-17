/// @file ScanManagerTest.cc
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
#include "casa/aips.h"
#include "configuration/Configuration.h"
#include "ConfigurationHelper.h"

// Classes to test
#include "ingestpipeline/sourcetask/ScanManager.h"

using namespace casa;

namespace askap {
namespace cp {
namespace ingest {

class ScanManagerTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(ScanManagerTest);
        CPPUNIT_TEST(testUpdate);
        CPPUNIT_TEST(testUpdateInsertInactiveMetadata);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        };

        void testUpdate() {
            testDriver(false);
        };

        void testUpdateInsertInactiveMetadata() {
            testDriver(true);
        };

    private:

        /// @param[in] insertInactiveMetadata   if true then between each update
        ///     indicating scan is active, an inactive update will be sent.
        void testDriver(bool insertInactiveMetadata) {
            ScanManager sm(ConfigurationHelper::createDummyConfig());
            CPPUNIT_ASSERT(!sm.observationComplete());
            CPPUNIT_ASSERT_EQUAL(-1L, sm.scanIndex());

            if (insertInactiveMetadata) {
                sm.update(false, "");
            }

            sm.update(true, "0");
            CPPUNIT_ASSERT(!sm.observationComplete());
            CPPUNIT_ASSERT_EQUAL(0L, sm.scanIndex());

            if (insertInactiveMetadata) {
                sm.update(false, "");
            }

            sm.update(true, "1");
            CPPUNIT_ASSERT(!sm.observationComplete());
            CPPUNIT_ASSERT_EQUAL(1L, sm.scanIndex());

            if (insertInactiveMetadata) {
                sm.update(false, "");
            }

            sm.update(true, "2");
            CPPUNIT_ASSERT(!sm.observationComplete());
            CPPUNIT_ASSERT_EQUAL(2L, sm.scanIndex());

            sm.update(false, "");
            CPPUNIT_ASSERT(sm.observationComplete());
        }

};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
