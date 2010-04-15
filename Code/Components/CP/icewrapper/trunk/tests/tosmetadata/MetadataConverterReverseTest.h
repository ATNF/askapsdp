/// @file MetadataConverterReverseTest.cc
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

// Classes to test
#include "tosmetadata/MetadataConverter.h"

// Using
using namespace casa;
using namespace askap::interfaces;

namespace askap {
namespace cp {

class MetadataConverterReverseTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(MetadataConverterReverseTest);
        CPPUNIT_TEST(testTime);
        CPPUNIT_TEST(testPeriod);
        CPPUNIT_TEST(testNBeams);
        CPPUNIT_TEST(testNCoarseChan);
        CPPUNIT_TEST(testNAntennas);
        CPPUNIT_TEST(testNPol);
        CPPUNIT_TEST(testAntennaNames);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
            // Initialize test values
            nCoarseChan = 304;
            nBeam = 36;
            nPol = 4;
            nAntenna = 1;
            timestamp = 1234567890;
            period = 5 * 1000 * 1000;

            // Setup the source object
            itsSource.reset(new TimeTaggedTypedValueMap);
            TypedValueMapMapper mapper(itsSource->data);

            // time
            mapper.setLong("time", timestamp);

            // period
            mapper.setLong("period", period);

            // n_coarse_chan
            mapper.setInt("n_coarse_chan", nCoarseChan);

            // n_antennas
            mapper.setInt("n_antennas", nAntenna);

            // n_beams
            std::vector<casa::Int> nBeamsVec;
            nBeamsVec.assign(nCoarseChan, nBeam);
            mapper.setIntSeq("n_beams", nBeamsVec);

            // n_pol
            mapper.setInt("n_pol", nPol);

            // Convert
            MetadataConverter converter;
            itsDest.reset(new TosMetadata(converter.convert(*itsSource)));
        }

        void tearDown()
        {
            itsSource.reset();
            itsDest.reset();
        }

        void testTime()
        {
            CPPUNIT_ASSERT_EQUAL(timestamp, static_cast<casa::Long>(itsDest->time()));
        }

        void testPeriod()
        {
        }

        void testNBeams()
        {
        }

        void testNCoarseChan()
        {
        }

        void testNAntennas()
        {
        }

        void testNPol()
        {
        }

        void testAntennaNames()
        {
        }

    private:
        // Support classes
        boost::scoped_ptr<TimeTaggedTypedValueMap> itsSource;
        boost::scoped_ptr<TosMetadata> itsDest;

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
