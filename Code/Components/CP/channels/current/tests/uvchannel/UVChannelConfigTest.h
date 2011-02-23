/// @file UVChannelConfigTest.cc
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

// System includes
#include <sstream>

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// ASKAPsoft includes
#include "Common/ParameterSet.h"

// Classes to test
#include "uvchannel/UVChannelConfig.h"

// Using
using namespace std;

namespace askap {
namespace cp {
namespace channels {

class UVChannelConfigTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(UVChannelConfigTest);
        CPPUNIT_TEST(testSingleBroker);
        CPPUNIT_TEST(testMultiBroker);
        CPPUNIT_TEST_SUITE_END();

    public:

        void setUp()
        {
        };

        void tearDown()
        {
        }

        void testSingleBroker() {
            LOFAR::ParameterSet parset;
            parset.add("uvchannel.brokers", "[broker1]");
            parset.add("uvchannel.broker.broker1.host", "localhost");
            parset.add("uvchannel.broker.broker1.port", "61616");
            parset.add("uvchannel.channels", "[avg304]");

            parset.add("uvchannel.channel.avg304.topic_prefix", "avg304");
            parset.add("uvchannel.channel.avg304.nblocks", "1");
            parset.add("uvchannel.channel.avg304.block_1", "[1, 304, broker1]");

            UVChannelConfig instance(parset);

            for (int i = 1; i <= 304; ++i) {
                const string brokerId = instance.getBrokerId("avg304", i);
                CPPUNIT_ASSERT_EQUAL(string("localhost"), instance.getHost(brokerId));
                CPPUNIT_ASSERT_EQUAL(61616, instance.getPort(brokerId));
                stringstream ss;
                ss << "avg304" << "_" << i;
                CPPUNIT_ASSERT_EQUAL(ss.str(), instance.getTopic("avg304", i));
            }
        }
        
        void testMultiBroker() {
            LOFAR::ParameterSet parset;
            parset.add("uvchannel.brokers", "[broker1, broker2]");
            parset.add("uvchannel.broker.broker1.host", "host1");
            parset.add("uvchannel.broker.broker1.port", "1234");
            
            parset.add("uvchannel.broker.broker2.host", "host2");
            parset.add("uvchannel.broker.broker2.port", "2345");

            parset.add("uvchannel.channels", "[full]");
            parset.add("uvchannel.channel.full.topic_prefix", "full");
            parset.add("uvchannel.channel.full.nblocks", "2");
            parset.add("uvchannel.channel.full.block_1", "[1, 8208, broker1]");
            parset.add("uvchannel.channel.full.block_2", "[8209, 16416, broker2]");

            UVChannelConfig instance(parset);

            for (int i = 1; i <= 8208; ++i) {
                const string brokerId = instance.getBrokerId("full", i);
                CPPUNIT_ASSERT_EQUAL(string("host1"), instance.getHost(brokerId));
                CPPUNIT_ASSERT_EQUAL(1234, instance.getPort(brokerId));
            }
            for (int i = 8209; i <= 16416; ++i) {
                const string brokerId = instance.getBrokerId("full", i);
                CPPUNIT_ASSERT_EQUAL(string("host2"), instance.getHost(brokerId));
                CPPUNIT_ASSERT_EQUAL(2345, instance.getPort(brokerId));
                stringstream ss;
                ss << "full" << "_" << i;
                CPPUNIT_ASSERT_EQUAL(ss.str(), instance.getTopic("full", i));
            }
        }
};

}   // End namespace channels

}   // End namespace cp

}   // End namespace askap
