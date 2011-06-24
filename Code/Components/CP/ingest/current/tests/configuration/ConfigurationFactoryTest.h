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

// Classes to test
#include "configuration/ConfigurationFactory.h"

namespace askap {
namespace cp {
namespace ingest {

class ConfigurationFactoryTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(ConfigurationFactoryTest);
        CPPUNIT_TEST(testFactory);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            // Array name
            itsParset.add("arrayname", "ASKAP");

            // Correlator modes
            itsParset.add("correlator.modes", "[StandardMode]");
            itsParset.add("correlator.mode.StandardMode.n_chan", "16416");
            itsParset.add("correlator.mode.StandardMode.chan_width", "18.51851851kHz");
            itsParset.add("correlator.mode.StandardMode.stokes", "[XX, XY, YX, YY]");

            // Feed configurations
            itsParset.add("feeds.names", "[SPF, PAF4]");

            itsParset.add("feeds.SPF.n_feeds", "1");
            itsParset.add("feeds.SPF.spacing", "0");
            itsParset.add("feeds.SPF.feed0", "[0.0, 0.0]");

            itsParset.add("feeds.PAF4.n_feeds", "4");
            itsParset.add("feeds.PAF4.spacing", "1deg");
            itsParset.add("feeds.PAF4.feed0", "[-0.5, 0.5]");
            itsParset.add("feeds.PAF4.feed1", "[0.5, 0.5]");
            itsParset.add("feeds.PAF4.feed2", "[-0.5, -0.5]");
            itsParset.add("feeds.PAF4.feed3", "[0.5, -0.5]");


            // Antennas
            itsParset.add("antennas.names", "[A0, A1, A2, A3, A4, A5]");

            itsParset.add("antennas.A0.location" , "");
            itsParset.add("antennas.A0.diameter" , "12m");
            itsParset.add("antennas.A0.mount" , "equatorial");
            itsParset.add("antennas.A1.feed_config" , "PAF4");


            // Observation specific
            itsParset.add("observation.sbid", "0");
            itsParset.add("observation.scan0.field_name", "test-field");
            itsParset.add("observation.scan0.field_direction", "[12h30m00.000, -45.00.00.000, J2000]");
            itsParset.add("observation.scan0.centre_freq", "1.420GHz");
            itsParset.add("observation.scan0.correlator_mode", "StandardMode");

            // Metadata topic config
            itsParset.add("metadata_source.ice.locator_host", "localhost");
            itsParset.add("metadata_source.ice.locator_port", "4061");
            itsParset.add("metadata_source.icestorm.topicmanager", "TopicManager");
            itsParset.add("metadata_source.icestorm.topic", "tosmetadata");

            // Calibration data service config
            itsParset.add("cal_data_service.ice.locator_host", "localhost");
            itsParset.add("cal_data_service.ice.locator_port", "4061");
            itsParset.add("cal_data_service.servicename", "CalibrationDataService");
        };

        void tearDown() {
            itsParset.clear();
        }

        void testFactory() {
            Configuration conf = ConfigurationFactory::createConfiguraton(itsParset);

            // Check array name
            CPPUNIT_ASSERT(conf.arrayName().compare("ASKAP") == 0);
        };

    private:
        LOFAR::ParameterSet itsParset;

};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap
