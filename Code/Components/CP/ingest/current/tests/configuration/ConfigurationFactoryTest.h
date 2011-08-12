/// @file ConfigurationFactoryTest.cc
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
#include "Common/ParameterSet.h"
#include "configuration/Configuration.h"
#include "casa/BasicSL.h"

// Classes to test
#include "configuration/ConfigurationFactory.h"

namespace askap {
namespace cp {
namespace ingest {

class ConfigurationFactoryTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(ConfigurationFactoryTest);
        CPPUNIT_TEST(testCreateConfiguration);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            // Array name
            itsParset.add("arrayname", "ASKAP");


            // Feed configurations
            itsParset.add("feeds.names", "[SPF, PAF4]");

            itsParset.add("feeds.SPF.n_feeds", "1");
            itsParset.add("feeds.SPF.spacing", "1deg");
            itsParset.add("feeds.SPF.feed0", "[0.0, 0.0]");

            itsParset.add("feeds.PAF4.n_feeds", "4");
            itsParset.add("feeds.PAF4.spacing", "1deg");
            itsParset.add("feeds.PAF4.feed0", "[-0.5, 0.5]");
            itsParset.add("feeds.PAF4.feed1", "[0.5, 0.5]");
            itsParset.add("feeds.PAF4.feed2", "[-0.5, -0.5]");
            itsParset.add("feeds.PAF4.feed3", "[0.5, -0.5]");


            // Antennas
            itsParset.add("antennas.names", "[A0, A1]");

            itsParset.add("antennas.A0.location" , "[-175.233429,  -1673.460938,  0.0000]");
            itsParset.add("antennas.A0.diameter" , "12m");
            itsParset.add("antennas.A0.mount" , "equatorial");
            itsParset.add("antennas.A0.feed_config" , "PAF4");

            itsParset.add("antennas.A1.location" , "[-175.233429,  -1673.460938,  0.0000]");
            itsParset.add("antennas.A1.diameter" , "15m");
            itsParset.add("antennas.A1.mount" , "equatorial");
            itsParset.add("antennas.A1.feed_config" , "SPF");

            // Observation specific
            itsParset.add("observation.sbid", "0");
            itsParset.add("observation.scan0.field_name", "test-field");
            itsParset.add("observation.scan0.field_direction", "[12h30m00.000, -45.00.00.000, J2000]");
            itsParset.add("observation.scan0.start_freq", "1.420GHz");
            itsParset.add("observation.scan0.n_chan", "16416");
            itsParset.add("observation.scan0.chan_width", "18.51851851kHz");
            itsParset.add("observation.scan0.stokes", "[XX, XY, YX, YY]");


            // Metadata topic config
            itsParset.add("metadata_source.ice.locator_host", "localhost");
            itsParset.add("metadata_source.ice.locator_port", "4061");
            itsParset.add("metadata_source.icestorm.topicmanager", "TopicManager");
            itsParset.add("metadata_source.icestorm.topic", "tosmetadata");

            // Calibration data service config
            itsParset.add("cal_data_service.ice.locator_host", "localhost");
            itsParset.add("cal_data_service.ice.locator_port", "4061");
            itsParset.add("cal_data_service.servicename", "CalibrationDataService");

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
            itsParset.add("tasks.MSSink.params.stman.bucketsize", "1048576");
            itsParset.add("tasks.MSSink.params.stman.tilencorr", "4");
            itsParset.add("tasks.MSSink.params.stman.tilenchan", "1");
        };

        void tearDown() {
            itsParset.clear();
        }

        void testCreateConfiguration() {
            Configuration conf = ConfigurationFactory::createConfiguraton(itsParset);

            // Check array name
            CPPUNIT_ASSERT_EQUAL(casa::String("ASKAP"), conf.arrayName());

            // Check observation
            const Observation obs = conf.observation();
            CPPUNIT_ASSERT_EQUAL(0u, obs.schedulingBlockID());

            // Check Scans
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), obs.scans().size());
            Scan s = obs.scans().at(0);
            CPPUNIT_ASSERT_EQUAL(casa::String("test-field"), s.name());
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(1.420, "GHz"), s.startFreq());
            CPPUNIT_ASSERT_EQUAL(16416u, s.nChan());
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(18.51851851, "kHz"), s.chanWidth());
            CPPUNIT_ASSERT_EQUAL(4ul, s.stokes().size());

            // Check antennas
            CPPUNIT_ASSERT_EQUAL(2ul, conf.antennas().size());

            // A0
            unsigned int idx = 0;
            CPPUNIT_ASSERT_EQUAL(casa::String("A0"), conf.antennas().at(idx).name());
            CPPUNIT_ASSERT_EQUAL(casa::String("equatorial"), conf.antennas().at(idx).mount());
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(12, "m"), conf.antennas().at(idx).diameter());
            CPPUNIT_ASSERT_EQUAL(4u, conf.antennas().at(idx).feeds().nFeeds());
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(-0.5, "deg"),
                    conf.antennas().at(idx).feeds().offsetX(0));
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(0.5, "deg"),
                    conf.antennas().at(idx).feeds().offsetY(0));
            CPPUNIT_ASSERT_EQUAL(casa::String("X Y"), conf.antennas().at(0).feeds().pol(0));

            // A1
            idx = 1;
            CPPUNIT_ASSERT_EQUAL(casa::String("A1"), conf.antennas().at(idx).name());
            CPPUNIT_ASSERT_EQUAL(casa::String("equatorial"), conf.antennas().at(idx).mount());
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(15, "m"), conf.antennas().at(idx).diameter());
            CPPUNIT_ASSERT_EQUAL(1u, conf.antennas().at(idx).feeds().nFeeds());
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(0.0, "deg"),
                    conf.antennas().at(idx).feeds().offsetX(0));
            CPPUNIT_ASSERT_EQUAL(casa::Quantity(0.0, "deg"),
                    conf.antennas().at(idx).feeds().offsetY(0));
            CPPUNIT_ASSERT_EQUAL(casa::String("X Y"), conf.antennas().at(0).feeds().pol(0));

            // Check tasks
            CPPUNIT_ASSERT_EQUAL(4ul, conf.tasks().size());

            idx = 0;
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
        };

    private:
        LOFAR::ParameterSet itsParset;

};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
