/// @file TosMetadataTest.cc
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
#include "boost/scoped_ptr.hpp"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"

// Classes to test
#include "cpcommon/TosMetadata.h"

// Using
using namespace casa;

namespace askap {
namespace cp {

class TosMetadataTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(TosMetadataTest);
        CPPUNIT_TEST(testConstructor);
        CPPUNIT_TEST(testAddAntenna);
        CPPUNIT_TEST(testAddAntennaDuplicate);
        CPPUNIT_TEST(testTime);
        CPPUNIT_TEST(testScanId);
        CPPUNIT_TEST(testFlagged);
        CPPUNIT_TEST(testTargetName);
        CPPUNIT_TEST(testTargetDirection);
        CPPUNIT_TEST(testPhaseDirection);
        CPPUNIT_TEST(testCorrMode);
        CPPUNIT_TEST(testAntennaAccess);
        CPPUNIT_TEST(testAntennaInvalid);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            instance.reset(new TosMetadata());
        }

        void tearDown() {
            instance.reset();
        }

        void testConstructor() {
            CPPUNIT_ASSERT_EQUAL(0u, instance->nAntenna());
            CPPUNIT_ASSERT_EQUAL(0ul, instance->time());
        }

        void testAddAntenna() {
            const casa::uInt nAntenna = 36;

            for (casa::uInt i = 0; i < nAntenna; ++i) {
                CPPUNIT_ASSERT_EQUAL(i, instance->nAntenna());
                TosMetadataAntenna ant("ak" + utility::toString(i));
                instance->addAntenna(ant);
            }

            CPPUNIT_ASSERT_EQUAL(nAntenna, instance->nAntenna());
        };

        void testAddAntennaDuplicate() {
            TosMetadataAntenna ant1("ak01");
            instance->addAntenna(ant1);
            CPPUNIT_ASSERT_EQUAL(1u, instance->nAntenna());
            CPPUNIT_ASSERT_THROW(instance->addAntenna(ant1), askap::AskapError);
            CPPUNIT_ASSERT_EQUAL(1u, instance->nAntenna());

            TosMetadataAntenna ant2("ak01"); // Different instance, same name
            CPPUNIT_ASSERT_THROW(instance->addAntenna(ant2), askap::AskapError);
            CPPUNIT_ASSERT_EQUAL(1u, instance->nAntenna());
        }

        void testTime() {
            const uLong testVal = 1234;
            instance->time(testVal);
            CPPUNIT_ASSERT_EQUAL(testVal, instance->time());
        }

        void testScanId() {
            for (casa::Int i = -2; i < 10; ++i) {
                instance->scanId(i);
                CPPUNIT_ASSERT_EQUAL(i, instance->scanId());
            }
        }

        void testFlagged() {
            instance->flagged(true);
            CPPUNIT_ASSERT_EQUAL(true, instance->flagged());
            instance->flagged(false);
            CPPUNIT_ASSERT_EQUAL(false, instance->flagged());
        }

        void testTargetName() {
            CPPUNIT_ASSERT(instance->targetName() == "");
            instance->targetName("1934-638");
            CPPUNIT_ASSERT(instance->targetName() == "1934-638");
        }

        void testTargetDirection() {
            const MDirection dir(Quantity(187.5, "deg"),
                    MDirection::Ref(MDirection::J2000));
            instance->targetDirection(dir);
        }

        void testPhaseDirection() {
            const MDirection dir(Quantity(187.5, "deg"),
                    MDirection::Ref(MDirection::J2000));
            instance->phaseDirection(dir);
        }

        void testCorrMode() {
            CPPUNIT_ASSERT(instance->corrMode() == "");
            instance->corrMode("standard");
            CPPUNIT_ASSERT(instance->corrMode() == "standard");
        }

        void testAntennaAccess() {
            const casa::String ant1Name = "ak01";
            const casa::String ant2Name = "ak02";
            const TosMetadataAntenna a1(ant1Name);
            const TosMetadataAntenna a2(ant2Name);

            CPPUNIT_ASSERT_EQUAL(0u, instance->nAntenna());
            instance->addAntenna(a1);
            CPPUNIT_ASSERT_EQUAL(1u, instance->nAntenna());
            instance->addAntenna(a2);
            CPPUNIT_ASSERT_EQUAL(2u, instance->nAntenna());

            const TosMetadataAntenna& ant1 = instance->antenna(ant1Name);
            CPPUNIT_ASSERT_EQUAL(ant1Name, ant1.name());
            const TosMetadataAntenna& ant2 = instance->antenna(ant2Name);
            CPPUNIT_ASSERT_EQUAL(ant2Name, ant2.name());
        }

        void testAntennaInvalid() {
            TosMetadataAntenna ant("ak01");
            instance->addAntenna(ant);

            // Request an invalid antenna id (wrong name)
            CPPUNIT_ASSERT_THROW(instance->antenna(""), askap::AskapError);
            CPPUNIT_ASSERT_THROW(instance->antenna("ak2"), askap::AskapError);
        }

    private:
        // Instance of class under test
        boost::scoped_ptr<TosMetadata> instance;
};

}   // End namespace cp

}   // End namespace askap
