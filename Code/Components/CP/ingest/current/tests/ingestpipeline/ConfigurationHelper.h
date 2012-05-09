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
            parset.add("arrayname", "ASKAP");

            // Observation specific
            parset.add("observation.sbid", "0");

            // Scan0
            parset.add("observation.scan0.field_name", "test-field0");
            parset.add("observation.scan0.field_direction", "[12h30m00.000, -45.00.00.000, J2000]");
            parset.add("observation.scan0.start_freq", "1.400GHz");
            parset.add("observation.scan0.n_chan", "16416");
            parset.add("observation.scan0.chan_width", "18.51851851kHz");
            parset.add("observation.scan0.stokes", "[XX, XY, YX, YY]");

            // Scan1
            parset.add("observation.scan1.field_name", "test-field2");
            parset.add("observation.scan1.field_direction", "[12h30m00.000, -45.00.00.000, J2000]");
            parset.add("observation.scan1.start_freq", "1.400GHz");
            parset.add("observation.scan1.n_chan", "16416");
            parset.add("observation.scan1.chan_width", "18.51851851kHz");
            parset.add("observation.scan1.stokes", "[XX, XY, YX, YY]");

            // Scan2
            parset.add("observation.scan2.field_name", "test-field2");
            parset.add("observation.scan2.field_direction", "[12h30m00.000, -45.00.00.000, J2000]");
            parset.add("observation.scan2.start_freq", "1.400GHz");
            parset.add("observation.scan2.n_chan", "16416");
            parset.add("observation.scan2.chan_width", "18.51851851kHz");
            parset.add("observation.scan2.stokes", "[XX, XY, YX, YY]");

            // Feed configurations
            parset.add("feeds.names", "[PAF]");

            parset.add("feeds.PAF.n_feeds", "4");
            parset.add("feeds.PAF.spacing", "1deg");
            parset.add("feeds.PAF.feed0", "[-2.5, -1.5]");
            parset.add("feeds.PAF.feed1", "[-2.5, -0.5]");
            parset.add("feeds.PAF.feed2", "[-2.5, 0.5]");
            parset.add("feeds.PAF.feed3", "[-2.5, 1.5]");

            // Antennas
            parset.add("antennas.names", "[A0, A1, A2, A3, A4, A5]");

            parset.add("antennas.A0.location" , "[-2652616.854602326, 5102312.637997697, -2749946.411592145]");
            parset.add("antennas.A0.diameter" , "12m");
            parset.add("antennas.A0.mount" , "equatorial");
            parset.add("antennas.A0.feed_config" , "PAF");

            parset.add("antennas.A1.location" , "[-2653178.349042055, 5102446.673161191, -2749155.53718417]");
            parset.add("antennas.A1.diameter" , "12m");
            parset.add("antennas.A1.mount" , "equatorial");
            parset.add("antennas.A1.feed_config" , "PAF");

            parset.add("antennas.A2.location" , "[-2652931.204894244, 5102600.67778301, -2749108.177002157]");
            parset.add("antennas.A2.diameter" , "12m");
            parset.add("antennas.A2.mount" , "equatorial");
            parset.add("antennas.A2.feed_config" , "PAF");

            parset.add("antennas.A3.location" , "[-2652731.709913884, 5102780.937978324, -2748966.073105379]");
            parset.add("antennas.A3.diameter" , "12m");
            parset.add("antennas.A3.mount" , "equatorial");
            parset.add("antennas.A3.feed_config" , "PAF");

            parset.add("antennas.A4.location" , "[-2652803.638192114, 5102632.431992128, -2749172.362663322]");
            parset.add("antennas.A4.diameter" , "12m");
            parset.add("antennas.A4.mount" , "equatorial");
            parset.add("antennas.A4.feed_config" , "PAF");

            parset.add("antennas.A5.location" , "[-2652492.544738157, 5102823.769989723, -2749117.418823366]");
            parset.add("antennas.A5.diameter" , "12m");
            parset.add("antennas.A5.mount" , "equatorial");
            parset.add("antennas.A5.feed_config" , "PAF");

            return Configuration(parset);
        }
};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap

#endif
