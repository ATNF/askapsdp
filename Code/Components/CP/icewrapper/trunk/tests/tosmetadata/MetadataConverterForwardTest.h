/// @file MetadataConverterForwardTest.cc
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

// System includes
#include <string>
#include <sstream>
#include <vector>

// Support classes
#include "boost/scoped_ptr.hpp"
#include "TypedValues.h"
#include "cpcommon/TosMetadata.h"
#include "tosmetadata/TypedValueMapMapper.h"
#include "casa/aips.h"

// Classes to test
#include "tosmetadata/MetadataConverter.h"

// Using
using namespace casa;
using namespace askap::interfaces;

namespace askap {
    namespace cp {

        class MetadataConverterForwardTest : public CppUnit::TestFixture {
            CPPUNIT_TEST_SUITE(MetadataConverterForwardTest);
            CPPUNIT_TEST(testTime);
            CPPUNIT_TEST(testPeriod);
            CPPUNIT_TEST(testNBeams);
            CPPUNIT_TEST(testNCoarseChan);
            CPPUNIT_TEST(testNAntennas);
            CPPUNIT_TEST(testNPol);
            CPPUNIT_TEST(testAntennaNames);
            CPPUNIT_TEST_SUITE_END();

            public:
            void setUp() {
                // Initialize test values
                nCoarseChan = 304;
                nBeam = 36;
                nPol = 4;
                nAntenna = 1;
                timestamp = 1234567890;
                period = 5 * 1000 * 1000;

                // Setup the source object
                itsSource.reset(new TosMetadata(nCoarseChan, nBeam, nPol));

                // Time
                itsSource->time(timestamp);

                // Period
                itsSource->period(period);

                // Antennas
                std::vector<std::string> antennaNames;
                for (int i = 0; i < nAntenna; ++i) {
                    std::stringstream ss;
                    ss << "ASKAP" << i;
                    itsSource->addAntenna(ss.str());
                    antennaNames.push_back(ss.str());
                }

                // Convert
                MetadataConverter converter;
                TimeTaggedTypedValueMap timeTaggedMap(converter.convert(*itsSource));
                CPPUNIT_ASSERT(timeTaggedMap.timestamp == timestamp);
                TypedValueMap& data = timeTaggedMap.data;
                itsMapper.reset(new TypedValueMapMapper(data));
            }

            void tearDown() {
                itsSource.reset();
                itsMapper.reset();
            }

            void testTime() {
                CPPUNIT_ASSERT(itsMapper->getLong("time") == timestamp);
            }

            void testPeriod() {
                CPPUNIT_ASSERT(itsMapper->getLong("period") == period);
            }

            void testNBeams() {
                //CPPUNIT_ASSERT(itsMapper->getLong("n_beams") == nBeams);
            }

            void testNCoarseChan() {
                CPPUNIT_ASSERT(itsMapper->getInt("n_coarse_chan") == nCoarseChan);
            }

            void testNAntennas() {
                CPPUNIT_ASSERT(itsMapper->getInt("n_antennas") == nAntenna);
            }

            void testNPol() {
                CPPUNIT_ASSERT(itsMapper->getInt("n_pol") == nPol);
            }

            void testAntennaNames() {
                std::vector<casa::String> names = itsMapper->getStringSeq("antenna_names");
                CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(nAntenna),
                        static_cast<unsigned int>(names.size()));
            }


            private:
            bool valExists(const std::string& key, const TypedValueMap& map)
            {
                return (map.count(key) == 1);
            }

            // Support classes
            boost::scoped_ptr<TosMetadata> itsSource;
            boost::scoped_ptr<TypedValueMapMapper> itsMapper;

            // Some constants
            casa::Int nCoarseChan;
            casa::Int nBeam;
            casa::Int nPol;
            casa::Int nAntenna;
            casa::Long timestamp;
            casa::Long period;
        };

    }   // End namespace cp

}   // End namespace askap
