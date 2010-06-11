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
        CPPUNIT_TEST(testTime);
        CPPUNIT_TEST(testPeriod);
        CPPUNIT_TEST(testAntennaAccess);
        CPPUNIT_TEST(testAntennaInvalid);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            const casa::uInt nCoarseChannels = 304;
            const casa::uInt nBeams = 36;
            const casa::uInt nPol = 4;
            instance.reset(new TosMetadata(nCoarseChannels, nBeams, nPol));
        }

        void tearDown() {
            instance.reset();
        }

        void testConstructor() {
            CPPUNIT_ASSERT_EQUAL(0u, instance->nAntenna());
            CPPUNIT_ASSERT_EQUAL(0ul, instance->time());
            CPPUNIT_ASSERT_EQUAL(0ul, instance->period());
        }

        void testAddAntenna() {
            const casa::uInt nAntenna = 36;

            for (casa::uInt i = 0; i < nAntenna; ++i) {
                CPPUNIT_ASSERT_EQUAL(i, instance->nAntenna());
                std::stringstream ss;
                ss << "ASKAP" << i;
                instance->addAntenna(ss.str());
            }

            CPPUNIT_ASSERT_EQUAL(nAntenna, instance->nAntenna());
        };

        void testTime() {
            const uLong testVal = 1234;
            instance->time(testVal);
            CPPUNIT_ASSERT_EQUAL(testVal, instance->time());
        }

        void testPeriod() {
            const uLong testVal = 5678;
            instance->period(testVal);
            CPPUNIT_ASSERT_EQUAL(testVal, instance->period());
        }

        void testAntennaAccess() {
            const casa::String ant1Name = "ASKAP01";
            const casa::String ant2Name = "ASKAP02";
            uInt id1 = instance->addAntenna(ant1Name);
            uInt id2 = instance->addAntenna(ant2Name);

            TosMetadataAntenna& ant1 = instance->antenna(id1);
            CPPUNIT_ASSERT_EQUAL(ant1Name, ant1.name());
            TosMetadataAntenna& ant2 = instance->antenna(id2);
            CPPUNIT_ASSERT_EQUAL(ant2Name, ant2.name());
        }

        void testAntennaInvalid() {
            // Request an invalid antenna id (index out of bounds).
            try {
                instance->antenna(0);
                CPPUNIT_FAIL("Expected exception to be thrown");
            } catch (AskapError&) {
                // This is good
            }
        }

    private:
        // Instance of class under test
        boost::scoped_ptr<TosMetadata> instance;
};

}   // End namespace cp

}   // End namespace askap
