/// @file TosMetadataAntennaTest.cc
///
/// @copyright (c) 2010 CSIRO
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
#include "boost/scoped_ptr.hpp"
#include "measures/Measures/MDirection.h"
#include "askap/AskapError.h"
#include "casa/Quanta/Quantum.h"

// Classes to test
#include "cpcommon/TosMetadataAntenna.h"

// Using
using namespace casa;

namespace askap {
namespace cp {

class TosMetadataAntennaTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(TosMetadataAntennaTest);
        CPPUNIT_TEST(testName);
        CPPUNIT_TEST(testActualRaDec);
        CPPUNIT_TEST(testActualAzEl);
        CPPUNIT_TEST(testPolAngle);
        CPPUNIT_TEST(testOnSource);
        CPPUNIT_TEST(testHwError);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            instance.reset(new TosMetadataAntenna("ak01"));
        }

        void tearDown() {
            instance.reset();
        }

        void testName() {
            const casa::String antennaName("ak01");
            CPPUNIT_ASSERT_EQUAL(antennaName, instance->name());
        };

        void testActualRaDec() {
            MDirection testDir(Quantity(20, "deg"),
                               Quantity(-10, "deg"),
                               MDirection::Ref(MDirection::J2000));

            instance->actualRaDec(testDir); // Set
            CPPUNIT_ASSERT(directionsEqual(testDir,
                                           instance->actualRaDec()));
        }

        void testActualAzEl() {
            MDirection testDir(Quantity(90, "deg"),
                               Quantity(45, "deg"),
                               MDirection::Ref(MDirection::AZEL));

            instance->actualAzEl(testDir); // Set
            CPPUNIT_ASSERT(directionsEqual(testDir,
                                           instance->actualAzEl()));
        }

        void testPolAngle() {
            const Quantity testVal = Quantity(1.123456, "rad");
            instance->actualPolAngle(testVal);
            CPPUNIT_ASSERT(testVal.getValue("rad") == instance->actualPolAngle().getValue("rad"));
        }

        void testOnSource() {
            instance->onSource(true);
            CPPUNIT_ASSERT_EQUAL(true, instance->onSource());
            instance->onSource(false);
            CPPUNIT_ASSERT_EQUAL(false, instance->onSource());
        }

        void testHwError() {
            instance->onSource(true);
            CPPUNIT_ASSERT_EQUAL(true, instance->onSource());
            instance->onSource(false);
            CPPUNIT_ASSERT_EQUAL(false, instance->onSource());
        }

    private:

        // Compare two MDirection instances, returning true if they are
        // equal, otherwise false
        bool directionsEqual(const MDirection& dir1, const MDirection& dir2) {
            bool equal = true;

            for (int i = 0; i < 2; ++i) {
                if (dir1.getAngle().getValue()(i) !=
                        dir2.getAngle().getValue()(i)) {
                    equal = false;
                }
            }

            return equal;
        }

        // Instance of class under test
        boost::scoped_ptr<TosMetadataAntenna> instance;
};

}   // End namespace cp
}   // End namespace askap
