/// @file ChannelManagerTest.cc
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
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "askap/AskapUtil.h"

// Classes to test
#include "ingestpipeline/sourcetask/ChannelManager.h"

using namespace casa;

namespace askap {
namespace cp {
namespace ingest {

class ChannelManagerTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(ChannelManagerTest);
        CPPUNIT_TEST(testLocalNChannels);
        CPPUNIT_TEST(testLocalFrequencies);
        CPPUNIT_TEST(testLocalFrequenciesBETA);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        };

        void testLocalNChannels() {
            LOFAR::ParameterSet params;
            params.add("n_channels.0", "256");
            params.add("n_channels.1", "512");

            ChannelManager cman(params);
            CPPUNIT_ASSERT_EQUAL(256u, cman.localNChannels(0));
            CPPUNIT_ASSERT_EQUAL(512u, cman.localNChannels(1));

            CPPUNIT_ASSERT_THROW(cman.localNChannels(2), askap::AskapError);
        };

        void testLocalFrequencies() {
            LOFAR::ParameterSet params;
            params.add("n_channels.0", "2");
            params.add("n_channels.1", "4");
            const int totalNChan = 6;

            const double centreFreq = 1.6;
            const double chanWidth = 0.2;
            const double tolerance = 1e-15;

            ChannelManager cman(params);
            casa::Vector<casa::Double> f0 = cman.localFrequencies(0, centreFreq, chanWidth, totalNChan);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), f0.size());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.1, f0(0), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.3, f0(1), tolerance);

            casa::Vector<casa::Double> f1 = cman.localFrequencies(1, centreFreq, chanWidth, totalNChan);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), f1.size());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, f1(0), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.7, f1(1), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.9, f1(2), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2.1, f1(3), tolerance);

            CPPUNIT_ASSERT_THROW(cman.localFrequencies(2, centreFreq, chanWidth, totalNChan),
                    askap::AskapError);
        };

        void testLocalFrequenciesBETA() {
            LOFAR::ParameterSet params;
            params.add("n_channels.0", "16416");
            const int nChan = 16416;

            const double centreFreq = 864;
            const double chanWidth = -(1.0/54.0);
            const double tolerance = 1e-2;

            ChannelManager cman(params);
            casa::Vector<casa::Double> f0 = cman.localFrequencies(0, centreFreq, chanWidth, nChan);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16416), f0.size());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1015.99, f0(0), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(712.009, f0(16415), tolerance);
        }

    private:

};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
