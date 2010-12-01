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

// Classes to test
#include "cpcommon/TosMetadataAntenna.h"

// Using
using namespace casa;

namespace askap {
namespace cp {

class TosMetadataAntennaTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(TosMetadataAntennaTest);
        CPPUNIT_TEST(testConstructor);
        CPPUNIT_TEST(testTargetRaDec);
        CPPUNIT_TEST(testFrequency);
        CPPUNIT_TEST(testClientId);
        CPPUNIT_TEST(testScanId);
        CPPUNIT_TEST(testPhaseTrackingCentre);
        CPPUNIT_TEST(testPolarisationOffset);
        CPPUNIT_TEST(testOnSource);
        CPPUNIT_TEST(testHwError);
        CPPUNIT_TEST(testFlagDetailed);
        CPPUNIT_TEST(testSystemTemp);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            const casa::String antennaName("ASKAP01");
            const casa::uInt nCoarseChannels = 304;
            const casa::uInt nBeams = 36;
            const casa::uInt nPol = 4;
            instance.reset(new TosMetadataAntenna(antennaName, nCoarseChannels, nBeams, nPol));
        }

        void tearDown() {
            instance.reset();
        }

        void testConstructor() {
            const casa::String antennaName("ASKAP01");
            const casa::uInt nCoarseChannels = 304;
            const casa::uInt nBeams = 36;
            const casa::uInt nPol = 4;
            // Check the parameters have been passed through
            CPPUNIT_ASSERT_EQUAL(antennaName, instance->name());
            CPPUNIT_ASSERT_EQUAL(nCoarseChannels, instance->nCoarseChannels());
            CPPUNIT_ASSERT_EQUAL(nBeams, instance->nBeams());
            CPPUNIT_ASSERT_EQUAL(nPol, instance->nPol());
        };

        void testTargetRaDec() {
            MDirection testDir(Quantity(20, "deg"),
                               Quantity(-10, "deg"),
                               MDirection::Ref(MDirection::J2000));

            instance->targetRaDec(testDir); // Set
            CPPUNIT_ASSERT(directionsEqual(testDir,
                                           instance->targetRaDec()));
        }

        void testFrequency() {
            const Double testVal = 1.0;
            instance->frequency(testVal);
            CPPUNIT_ASSERT(testVal == instance->frequency());
        }

        void testClientId() {
            const String testVal = "test";
            instance->clientId(testVal);
            CPPUNIT_ASSERT_EQUAL(testVal, instance->clientId());
        }

        void testScanId() {
            const String testVal = "test";
            instance->scanId(testVal);
            CPPUNIT_ASSERT_EQUAL(testVal, instance->scanId());
        }

        void testPhaseTrackingCentre() {
            MDirection testDir(Quantity(20, "deg"),
                               Quantity(-10, "deg"),
                               MDirection::Ref(MDirection::J2000));

            for (uInt beam = 0; beam < instance->nBeams(); ++beam) {
                for (uInt chan = 0; chan < instance->nCoarseChannels(); ++chan) {
                    instance->phaseTrackingCentre(testDir, beam); // Set

                    // Check
                    CPPUNIT_ASSERT(directionsEqual(testDir,
                                instance->phaseTrackingCentre(beam)));
                }
            }

            // Request an invalid beam (index out of bounds)
            // Ask for beam 36 where range is 0..35
            CPPUNIT_ASSERT_THROW(
                    instance->phaseTrackingCentre(instance->nBeams() + 1),
                    askap::AskapError);
        };

        void testPolarisationOffset() {
            const Double testVal = 1.123456;
            instance->polarisationOffset(testVal);
            CPPUNIT_ASSERT(testVal == instance->polarisationOffset());
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

        void testFlagDetailed() {
            for (uInt beam = 0; beam < instance->nBeams(); ++beam) {
                for (uInt chan = 0; chan < instance->nCoarseChannels(); ++chan) {
                    for (uInt pol = 0; pol < instance->nPol(); ++pol) {
                        // Should be initialised to false
                        CPPUNIT_ASSERT_EQUAL(false, instance->flagDetailed(beam, chan, pol));

                        // Set to true then check
                        instance->flagDetailed(true, beam, chan, pol);
                        CPPUNIT_ASSERT_EQUAL(true, instance->flagDetailed(beam, chan, pol));
                    }
                }
            }
        }

        void testSystemTemp() {
            Float testVal = 12.0;

            for (uInt beam = 0; beam < instance->nBeams(); ++beam) {
                for (uInt chan = 0; chan < instance->nCoarseChannels(); ++chan) {
                    for (uInt pol = 0; pol < instance->nPol(); ++pol) {
                        // Should be initialised to -1.0
                        CPPUNIT_ASSERT(-1.0 == instance->systemTemp(beam, chan, pol));

                        instance->systemTemp(testVal, beam, chan, pol);
                        CPPUNIT_ASSERT(testVal == instance->systemTemp(beam, chan, pol));
                    }
                }
            }
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
