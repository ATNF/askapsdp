/// @file ConfigurationTest.cc
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
#include <limits>
#include "Common/ParameterSet.h"
#include "casa/BasicSL.h"

// Classes to test
#include "configuration/Configuration.h"

namespace askap {
namespace cp {
namespace ingest {

class ConfigurationTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(ConfigurationTest);
        CPPUNIT_TEST(testArrayName);
        CPPUNIT_TEST(testSchedulingBlockID);
        CPPUNIT_TEST(testTasks);
        CPPUNIT_TEST(testAntennas);
        CPPUNIT_TEST(testFeed);
        CPPUNIT_TEST(testNScans);
        CPPUNIT_TEST(testGetTargetForScan);
        CPPUNIT_TEST(testServiceConfig);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            // Observation (from Scheduling block)
            itsParset.add("sb.id", "0");
            itsParset.add("sb.targets", "[src1, src2]");

            itsParset.add("sb.target.src1.field_name", "test-field1");
            itsParset.add("sb.target.src1.field_direction", "[12h30m00.000, -45d00m00.000, J2000]");
            itsParset.add("sb.target.src1.corrmode", "standard");

            itsParset.add("sb.target.src2.field_name", "test-field2");
            itsParset.add("sb.target.src2.field_direction", "[12h30m00.000, -45d00m00.000, J2000]");
            itsParset.add("sb.target.src2.phase_direction", "[0h00m00.000, -35d00m00.000, J2000]");
            itsParset.add("sb.target.src2.corrmode", "standard");

            // Array name
            itsParset.add("array.name", "ASKAP");

            // TOS metadata topic
            itsParset.add("metadata.topic", "metadata");

            // Feed configurations
            itsParset.add("feeds.n_feeds", "4");
            itsParset.add("feeds.spacing", "1deg");
            itsParset.add("feeds.feed0", "[-0.5, 0.5]");
            itsParset.add("feeds.feed1", "[0.5, 0.5]");
            itsParset.add("feeds.feed2", "[-0.5, -0.5]");
            itsParset.add("feeds.feed3", "[0.5, -0.5]");

            // Antennas
            itsParset.add("antennas", "[ant1, ant3, ant6, ant8, ant9, ant15]");

            itsParset.add("antenna.ant.diameter", "12m");
            itsParset.add("antenna.ant.mount", "equatorial");

            itsParset.add("antenna.ant1.name", "ak01");
            itsParset.add("antenna.ant1.location.itrf", "[-2556084.669, 5097398.337, -2848424.133]");

            itsParset.add("antenna.ant3.name", "ak03");
            itsParset.add("antenna.ant3.location.itrf", "[-2556118.102, 5097384.726, -2848417.280]");

            itsParset.add("antenna.ant6.name", "ak06");
            itsParset.add("antenna.ant6.location.itrf", "[-2556227.863, 5097380.399, -2848323.367]");

            itsParset.add("antenna.ant8.name", "ak08");
            itsParset.add("antenna.ant8.location.itrf", "[-2556002.713742, 5097320.608027, -2848637.727970]");

            itsParset.add("antenna.ant9.name", "ak09");
            itsParset.add("antenna.ant9.location.itrf", "[-2555888.9789, 5097552.500315, -2848324.911449]");

            itsParset.add("antenna.ant15.name", "ak15");
            itsParset.add("antenna.ant15.location.itrf", "[-2555389.70943903, 5097664.08452923, -2848561.871727]");

            itsParset.add("correlator.modes", "[standard]");
            itsParset.add("correlator.mode.standard.chan_width", "18.518518kHz");
            itsParset.add("correlator.mode.standard.interval", "5000000");
            itsParset.add("correlator.mode.standard.n_chan", "16416");
            itsParset.add("correlator.mode.standard.stokes", "[XX, XY, YX, YY]");

            // Metadata topic config
            itsParset.add("metadata_source.ice.locator_host", "localhost");
            itsParset.add("metadata_source.ice.locator_port", "4061");
            itsParset.add("metadata_source.icestorm.topicmanager", "TopicManager");

            // Baseline IDs
            itsParset.add("baselinemap.baselineids", "[0..2]");
            itsParset.add("baselinemap.antennaidx", "[ak06, ak01, ak03, ak15, ak08, ak09]");

            itsParset.add("baselinemap.0", "[0, 0, XX]");
            itsParset.add("baselinemap.1", "[0, 0, XY]");
            itsParset.add("baselinemap.2", "[0, 0, YY]");

            /////////////////////////////
            // Task Configuration
            /////////////////////////////
            itsParset.add("tasks.tasklist", "[MergedSource, CalcUVWTask, ChannelAvgTask, MSSink]");

            // MergedSource
            itsParset.add("tasks.MergedSource.type", "MergedSource");
            itsParset.add("tasks.MergedSource.params.vis_source.port", "3000");
            itsParset.add("tasks.MergedSource.params.vis_source.buffer_size", "459648");

            // CalcUVWTask
            itsParset.add("tasks.CalcUVWTask.type", "CalcUVWTask");

            // ChannelAvgTask
            itsParset.add("tasks.ChannelAvgTask.type", "ChannelAvgTask");
            itsParset.add("tasks.ChannelAvgTask.params.averaging", "54");

            // MSSink
            itsParset.add("tasks.MSSink.type", "MSSink");
            itsParset.add("tasks.MSSink.params.filenamebase", "ingest_test");
            itsParset.add("tasks.MSSink.params.stman.bucketsize", "65536");
            itsParset.add("tasks.MSSink.params.stman.tilencorr", "4");
            itsParset.add("tasks.MSSink.params.stman.tilenchan", "1026");
        };

        void tearDown() {
            itsParset.clear();
        }

        void testArrayName() {
            Configuration conf(itsParset);
            CPPUNIT_ASSERT_EQUAL(casa::String("ASKAP"), conf.arrayName());
        }

        void testSchedulingBlockID() {
            Configuration conf(itsParset);
            CPPUNIT_ASSERT_EQUAL(0u, conf.schedulingBlockID());
        }


        void testTasks() {
            Configuration conf(itsParset);

            CPPUNIT_ASSERT_EQUAL(4ul, conf.tasks().size());

            unsigned int idx = 0;
            CPPUNIT_ASSERT_EQUAL(std::string("MergedSource"), conf.tasks().at(idx).name());
            CPPUNIT_ASSERT(conf.tasks().at(idx).type() == TaskDesc::MergedSource);
            CPPUNIT_ASSERT_EQUAL(2, conf.tasks().at(idx).params().size());
            CPPUNIT_ASSERT(conf.tasks().at(idx).params().isDefined("vis_source.port"));
            CPPUNIT_ASSERT(conf.tasks().at(idx).params().isDefined("vis_source.buffer_size"));

            idx = 1;
            CPPUNIT_ASSERT_EQUAL(std::string("CalcUVWTask"), conf.tasks().at(idx).name());
            CPPUNIT_ASSERT(conf.tasks().at(idx).type() == TaskDesc::CalcUVWTask);
            CPPUNIT_ASSERT_EQUAL(0, conf.tasks().at(idx).params().size());

            idx = 2;
            CPPUNIT_ASSERT_EQUAL(std::string("ChannelAvgTask"), conf.tasks().at(idx).name());
            CPPUNIT_ASSERT(conf.tasks().at(idx).type() == TaskDesc::ChannelAvgTask);
            CPPUNIT_ASSERT_EQUAL(1, conf.tasks().at(idx).params().size());
            CPPUNIT_ASSERT(conf.tasks().at(idx).params().isDefined("averaging"));

            idx = 3;
            CPPUNIT_ASSERT_EQUAL(std::string("MSSink"), conf.tasks().at(idx).name());
            CPPUNIT_ASSERT(conf.tasks().at(idx).type() == TaskDesc::MSSink);
            CPPUNIT_ASSERT_EQUAL(4, conf.tasks().at(idx).params().size());
        }

        void testAntennas() {
            Configuration conf(itsParset);

            CPPUNIT_ASSERT_EQUAL(6ul, conf.antennas().size());

            // A0
            unsigned int idx = 0;
            CPPUNIT_ASSERT_EQUAL(casa::String("ak06"), conf.antennas().at(idx).name());
            CPPUNIT_ASSERT_EQUAL(casa::String("equatorial"), conf.antennas().at(idx).mount());
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(12, "m"), conf.antennas().at(idx).diameter());

            // A1
            idx = 1;
            CPPUNIT_ASSERT_EQUAL(casa::String("ak01"), conf.antennas().at(idx).name());
            CPPUNIT_ASSERT_EQUAL(casa::String("equatorial"), conf.antennas().at(idx).mount());
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(12, "m"), conf.antennas().at(idx).diameter());

            // A2
            idx = 2;
            CPPUNIT_ASSERT_EQUAL(casa::String("ak03"), conf.antennas().at(idx).name());
            CPPUNIT_ASSERT_EQUAL(casa::String("equatorial"), conf.antennas().at(idx).mount());
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(12, "m"), conf.antennas().at(idx).diameter());
        }

        void testFeed() {
            Configuration conf(itsParset);
            const FeedConfig& feed = conf.feed();
            CPPUNIT_ASSERT_EQUAL(4u, feed.nFeeds());

            CPPUNIT_ASSERT_EQUAL(casa::Quantity(-0.5, "deg"), feed.offsetX(0));
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(0.5, "deg"), feed.offsetY(0));

            CPPUNIT_ASSERT_EQUAL(casa::Quantity(0.5, "deg"), feed.offsetX(1));
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(0.5, "deg"), feed.offsetY(1));

            CPPUNIT_ASSERT_EQUAL(casa::Quantity(-0.5, "deg"), feed.offsetX(2));
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(-0.5, "deg"), feed.offsetY(2));

            CPPUNIT_ASSERT_EQUAL(casa::Quantity(0.5, "deg"), feed.offsetX(3));
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(-0.5, "deg"), feed.offsetY(3));

            CPPUNIT_ASSERT_EQUAL(casa::String("X Y"), feed.pol(0));
        }

        void testNScans() {
            Configuration conf(itsParset);
            CPPUNIT_ASSERT_EQUAL(2u, conf.nScans());
        }

        void testGetTargetForScan() {
            Configuration conf(itsParset);

            const Target& t0 = conf.getTargetForScan(0);
            CPPUNIT_ASSERT_EQUAL(casa::String("test-field1"), t0.name());
            const CorrelatorMode& c0 = t0.mode();
            CPPUNIT_ASSERT_EQUAL(16416u, c0.nChan());
            const double tolerance = std::numeric_limits<float>::epsilon();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-172.5, t0.pointingCentre().getAngle().getValue("deg")(0), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-45.0, t0.pointingCentre().getAngle().getValue("deg")(1), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-172.5, t0.phaseCentre().getAngle().getValue("deg")(0), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-45.0, t0.phaseCentre().getAngle().getValue("deg")(1), tolerance);

            const Target& t1 = conf.getTargetForScan(1);
            CPPUNIT_ASSERT_EQUAL(casa::String("test-field2"), t1.name());
            const CorrelatorMode& c1 = t1.mode();
            CPPUNIT_ASSERT_EQUAL(16416u, c1.nChan());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-172.5, t1.pointingCentre().getAngle().getValue("deg")(0), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-45.0, t1.pointingCentre().getAngle().getValue("deg")(1), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, t1.phaseCentre().getAngle().getValue("deg")(0), tolerance);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-35.0, t1.phaseCentre().getAngle().getValue("deg")(1), tolerance);
        }

        void testServiceConfig() {
            Configuration conf(itsParset);
        }

    private:
        LOFAR::ParameterSet itsParset;

};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
