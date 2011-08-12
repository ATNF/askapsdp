/// @file Configuration.cc
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

// Include own header file first
#include "Configuration.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "boost/scoped_ptr.hpp"
#include "casa/BasicSL.h"

// Local package includes
#include "configuration/TopicConfig.h"
#include "configuration/ServiceConfig.h"
#include "configuration/TaskDesc.h"
#include "configuration/Antenna.h"
#include "configuration/Observation.h"

using namespace askap;
using namespace askap::cp::ingest;

Configuration::Configuration(const casa::String& arrayName,
                             const std::vector<TaskDesc>& tasks,
                             const std::vector<Antenna>& antennas,
                             const Observation& observation,
                             const TopicConfig& metadataTopic,
                             const ServiceConfig& calibrationDataService)
        : itsArrayName(arrayName), itsTasks(tasks), itsAntennas(antennas),
        itsObservation(observation), itsMetadataTopicConfig(metadataTopic),
        itsCalibrationDataServiceConfig(calibrationDataService)
{
}

casa::String Configuration::arrayName(void) const
{
    return itsArrayName;
}

std::vector<TaskDesc> Configuration::tasks(void) const
{
    return itsTasks;
}

std::vector<Antenna> Configuration::antennas(void) const
{
    return itsAntennas;
}

Observation Configuration::observation(void) const
{
    return itsObservation;
}

TopicConfig Configuration::metadataTopic(void) const
{
    return itsMetadataTopicConfig;
}

ServiceConfig Configuration::calibrationDataService(void) const
{
    return itsCalibrationDataServiceConfig;
}
