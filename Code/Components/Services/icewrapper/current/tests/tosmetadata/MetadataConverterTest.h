/// @file MetadataConverterTest.cc
///
/// @copyright (c) 2010,2014 CSIRO
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

// System includes
#include <string>
#include <vector>
#include <limits>

// Support classes
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "boost/scoped_ptr.hpp"
#include "TypedValues.h"
#include "cpcommon/TosMetadata.h"
#include "casa/aips.h"
#include "measures/Measures/MDirection.h"

// Classes to test
#include "tosmetadata/MetadataConverter.h"

// Using
using namespace std;
using namespace casa;
using namespace askap::interfaces;

namespace askap {
namespace cp {
namespace icewrapper {

class MetadataConverterTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(MetadataConverterTest);
        CPPUNIT_TEST(testNonLP64);
#ifdef __LP64__
        CPPUNIT_TEST(testConverter);
        CPPUNIT_TEST(testConverterAntenna);
#endif
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            // Test values (TosMetadata)
            const casa::uInt nAntenna = 6;
            const casa::uLong timestamp = 1234567890;

            // Test values (TosMetadataAntenna)
            const MDirection testDir(Quantity(20, "deg"),
                                     Quantity(-10, "deg"),
                                     MDirection::Ref(MDirection::J2000));
            const casa::Int scanId = 0;
            const casa::Quantity polAngle(1.234567, "rad");
            const casa::Bool onSource = true;
            const casa::Bool flagged = false;
            const casa::Quantity centreFreq(1400.0, "MHz");
            const std::string targetName("1934-638");
            const std::string corrMode("Standard");

            //////////////////////////////////////
            // Setup the source TosMetadata object
            //////////////////////////////////////
            itsSource.reset(new TosMetadata());

            // Time
            itsSource->time(timestamp);

            // ScanId
            itsSource->scanId(scanId);

            // Centre Frequency
            itsSource->centreFreq(centreFreq);
        
            // Target name
            itsSource->targetName(targetName);

            // Targt Direction
            itsSource->targetDirection(testDir);

            // Phase Centre
            itsSource->phaseDirection(testDir);

            // Correlator mode
            itsSource->corrMode(corrMode);

            // Antennas
            for (casa::uInt i = 0; i < nAntenna; ++i) {
                const std::string name = "ak" + utility::toString(i);
                TosMetadataAntenna ant(name);
                ant.actualRaDec(testDir);
                ant.actualAzEl(testDir);
                ant.actualPolAngle(polAngle);
                ant.onSource(onSource);
                ant.flagged(flagged);

                itsSource->addAntenna(ant);
            }

#ifndef __LP64__

            try {
#endif
                ///////////////////////////////////////////////////
                // Convert (TosMetadata -> TimeTaggedTypedValueMap)
                ///////////////////////////////////////////////////
                MetadataConverter converter;
                TimeTaggedTypedValueMap intermediate(converter.convert(*itsSource));

                ////////////////////////////////
                // Convert back to a TosMetadata
                ////////////////////////////////
                itsResult.reset(new TosMetadata(converter.convert(intermediate)));
#ifndef __LP64__
                CPPUNIT_FAIL("Expected exception not thrown");
            } catch (askap::AskapError&) {
                // This is expected for 32-bit platforms
            }

#endif
        }

        void tearDown() {
            itsSource.reset();
            itsResult.reset();
        }

        void testNonLP64() {
            // This is here to ensure setUp() is attempted on 32-bit platforms.
            // The other tests are for LP64 platforms only.
        }

        void testConverter() {
            CPPUNIT_ASSERT_EQUAL(itsSource->nAntenna(), itsResult->nAntenna());
            CPPUNIT_ASSERT_EQUAL(itsSource->time(), itsResult->time());
            CPPUNIT_ASSERT_EQUAL(itsSource->scanId(), itsResult->scanId());
            CPPUNIT_ASSERT_EQUAL(itsSource->flagged(), itsResult->flagged());
            CPPUNIT_ASSERT_EQUAL(itsSource->targetName(), itsResult->targetName());
            CPPUNIT_ASSERT_EQUAL(itsSource->centreFreq(), itsResult->centreFreq());
            verifyDir(itsSource->targetDirection(), itsResult->targetDirection());
            verifyDir(itsSource->phaseDirection(), itsResult->phaseDirection());
            CPPUNIT_ASSERT_EQUAL(itsSource->corrMode(), itsResult->corrMode());
        }

        void testConverterAntenna() {
            const unsigned int nAntenna = itsSource->nAntenna();
            CPPUNIT_ASSERT_EQUAL(nAntenna, itsResult->nAntenna());

            const vector<string> names = itsSource->antennaNames();
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nAntenna), names.size());
            for (unsigned int i = 0; i < names.size(); ++i) {
                verifyAntenna(names[i]);
            }
        }

    private:

        void verifyAntenna(const std::string& name) {
            // Get the TosMetadataAntenna instance
            const TosMetadataAntenna& srcAnt = itsSource->antenna(name);
            const TosMetadataAntenna& resultAnt = itsResult->antenna(name);

            CPPUNIT_ASSERT_EQUAL(srcAnt.name(), resultAnt.name());

            verifyDir(srcAnt.actualRaDec(), resultAnt.actualRaDec());
            verifyDir(srcAnt.actualAzEl(), resultAnt.actualAzEl());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(srcAnt.actualPolAngle().getValue("rad"),
                resultAnt.actualPolAngle().getValue("rad"),
                std::numeric_limits<float>::epsilon());
            CPPUNIT_ASSERT_EQUAL(srcAnt.onSource(), resultAnt.onSource());
            CPPUNIT_ASSERT_EQUAL(srcAnt.flagged(), resultAnt.flagged());
        }

        void verifyDir(const casa::MDirection& d1, const casa::MDirection& d2) {
            CPPUNIT_ASSERT_EQUAL(d1.getAngle().getValue()(0), d2.getAngle().getValue()(0));
            CPPUNIT_ASSERT_EQUAL(d1.getAngle().getValue()(1), d2.getAngle().getValue()(1));
            CPPUNIT_ASSERT(d1.getRef().getType() == d2.getRef().getType());
        }

        // Support classes
        boost::scoped_ptr<TosMetadata> itsSource;
        boost::scoped_ptr<TosMetadata> itsResult;
};

}   // End namespace icewrapper

}   // End namespace cp

}   // End namespace askap
