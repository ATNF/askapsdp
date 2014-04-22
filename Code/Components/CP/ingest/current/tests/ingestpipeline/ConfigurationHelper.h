/// @file ConfigurationHelper.cc
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

#ifndef ASKAP_CP_INGEST_CONFIGURATIONHELPER_H
#define ASKAP_CP_INGEST_CONFIGURATIONHELPER_H

// System includes
#include <string>

// ASKAPsoft includes
#include "Common/ParameterSet.h"

// Local package includes
#include "configuration/Configuration.h"

using namespace casa;
using askap::cp::common::VisChunk;

namespace askap {
namespace cp {
namespace ingest {

class ConfigurationHelper {

    public:
        static Configuration createDummyConfig(void)
        {
            LOFAR::ParameterSet parset;

            // Array name
            parset.add("array.name", "ASKAP");

            // Metadata Topic
            parset.add("metadata.topic", "metadata");

            // Observation specific
            parset.add("sbid", "0");

            // Targets
            parset.add("targets", "[src1, src2, src3]");

            // Scan1
            parset.add("target.src1.field_name", "test-field1");
            parset.add("target.src1.field_direction", "[12h30m00.000, -45d00m00.000, J2000]");
            parset.add("target.src1.corrmode", "standard");
            
            // Scan2
            parset.add("target.src2.field_name", "test-field2");
            parset.add("target.src2.field_direction", "[12h30m00.000, -45d00m00.000, J2000]");
            parset.add("target.src2.corrmode", "standard");

            // Scan3
            parset.add("target.src3.field_name", "test-field3");
            parset.add("target.src3.field_direction", "[12h30m00.000, -45d00m00.000, J2000]");
            parset.add("target.src3.corrmode", "standard");

            // Correlator mode(s)
            parset.add("correlator.modes", "[standard]");

            parset.add("correlator.mode.standard.chan_width", "18.518518kHz");
            parset.add("correlator.mode.standard.interval", "5000000");
            parset.add("correlator.mode.standard.n_chan", "16416");
            parset.add("correlator.mode.standard.stokes", "[XX, XY, YX, YY]");

            // Feed configurations
            parset.add("feeds.n_feeds", "4");
            parset.add("feeds.spacing", "1deg");

            parset.add("feeds.feed0", "[-2.5, -1.5]");
            parset.add("feeds.feed1", "[-2.5, -0.5]");
            parset.add("feeds.feed2", "[-2.5, 0.5]");
            parset.add("feeds.feed3", "[-2.5, 1.5]");

            // Antennas
            parset.add("antennas", "[ant1, ant3, ant6, ant8, ant9, ant15]");

            parset.add("antenna.ant.diameter", "12m");
            parset.add("antenna.ant.mount", "equatorial");

            parset.add("antenna.ant1.name", "ak01");
            parset.add("antenna.ant1.location.itrf", "[-2556084.669, 5097398.337, -2848424.133]");

            parset.add("antenna.ant3.name", "ak03");
            parset.add("antenna.ant3.location.itrf", "[-2556118.102, 5097384.726, -2848417.280]");

            parset.add("antenna.ant6.name", "ak06");
            parset.add("antenna.ant6.location.itrf", "[-2556227.863, 5097380.399, -2848323.367]");

            parset.add("antenna.ant8.name", "ak08");
            parset.add("antenna.ant8.location.itrf", "[-2556002.713742, 5097320.608027, -2848637.727970]");

            parset.add("antenna.ant9.name", "ak09");
            parset.add("antenna.ant9.location.itrf", "[-2555888.9789, 5097552.500315, -2848324.911449]");

            parset.add("antenna.ant15.name", "ak15");
            parset.add("antenna.ant15.location.itrf", "[-2555389.70943903, 5097664.08452923, -2848561.871727]");

            // Baseline ID Map
            parset.add("baselinemap.baselineids","[1..21]");

            parset.add("baselinemap.antennaidx", "[ak06, ak01, ak03, ak15, ak08, ak09]");

            parset.add("baselinemap.1", "[0, 0, XX]");     
            parset.add("baselinemap.2", "[0, 0, XY]");     
            parset.add("baselinemap.3", "[0, 1, XX]");     
            parset.add("baselinemap.4", "[0, 1, XY]");     
            parset.add("baselinemap.5", "[0, 2, XX]");     
            parset.add("baselinemap.6", "[0, 2, XY]");     
            parset.add("baselinemap.7", "[0, 0, YY]");     
            parset.add("baselinemap.8", "[0, 1, YX]");     
            parset.add("baselinemap.9", "[0, 1, YY]");     
            parset.add("baselinemap.10", "[0, 2, YX]");    
            parset.add("baselinemap.11", "[0, 2, YY]");    

            parset.add("baselinemap.12", "[1, 1, XX]");    
            parset.add("baselinemap.13", "[1, 1, XY]");    
            parset.add("baselinemap.14", "[1, 2, XX]");    
            parset.add("baselinemap.15", "[1, 2, XY]");    
            parset.add("baselinemap.16", "[1, 1, YY]");    
            parset.add("baselinemap.17", "[1, 2, YX]");    
            parset.add("baselinemap.18", "[1, 2, YY]");    

            parset.add("baselinemap.19", "[2, 2, XX]");    
            parset.add("baselinemap.20", "[2, 2, XY]");    
            parset.add("baselinemap.21", "[2, 2, YY]");

            /////////////////////////////
            // Task Configuration
            /////////////////////////////
            parset.add("tasks.tasklist", "[MergedSource, CalcUVWTask, ChannelAvgTask, MSSink]");

            // MergedSource
            parset.add("tasks.MergedSource.type", "MergedSource");
            parset.add("tasks.MergedSource.params.vis_source.port", "3000");
            parset.add("tasks.MergedSource.params.vis_source.buffer_size", "459648");

            // CalcUVWTask
            parset.add("tasks.CalcUVWTask.type", "CalcUVWTask");

            // ChannelAvgTask
            parset.add("tasks.ChannelAvgTask.type", "ChannelAvgTask");
            parset.add("tasks.ChannelAvgTask.params.averaging", "54");

            // MSSink
            parset.add("tasks.MSSink.type", "MSSink");
            parset.add("tasks.MSSink.params.filenamebase", "ingest_test");
            parset.add("tasks.MSSink.params.stman.bucketsize", "65536");
            parset.add("tasks.MSSink.params.stman.tilencorr", "4");
            parset.add("tasks.MSSink.params.stman.tilenchan", "1026");

            return Configuration(parset);
        }
};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap

#endif
