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
#include <vector>
#include <map>

// ASKAPsoft includes
#include "casa/Quanta/Quantum.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/Stokes.h"

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
            const std::string arrayName;
            std::vector<TaskDesc> tasks;
            std::vector<Antenna> antennas;

            // Create Stokes vector for the scan configuration
            std::vector<casa::Stokes::StokesTypes> stokes;
            stokes.push_back(casa::Stokes::XX);
            stokes.push_back(casa::Stokes::XY);
            stokes.push_back(casa::Stokes::YX);
            stokes.push_back(casa::Stokes::YY);

            // An observation must have at least one scan, so add one
            Scan scan0("test-field",
                    casa::MDirection(casa::Quantity(187.5, "deg"),
                        casa::Quantity(-45.0, "deg"),
                        MDirection::J2000),
                    casa::Quantity(1400, "GHz"),
                    16416,
                    casa::Quantity(18.5, "kHz"),
                    stokes);
            std::vector<Scan> scans;
            scans.push_back(scan0);

            Observation observation(0, scans);
            TopicConfig metadataTopic("", "", "", "");
            ServiceConfig calibrationDataService("", "", "");

            return Configuration(arrayName, tasks, antennas, observation,
                    metadataTopic, calibrationDataService);
        }
};

}   // End namespace ingest
}   // End namespace cp
}   // End namespace askap

#endif
