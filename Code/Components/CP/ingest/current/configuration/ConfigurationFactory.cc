/// @file ConfigurationFactory.cc
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
#include "ConfigurationFactory.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <limits>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "casa/BasicSL.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "configuration/TopicConfig.h"
#include "configuration/ServiceConfig.h"
#include "configuration/TaskDesc.h"
#include "configuration/Antenna.h"
#include "configuration/CorrelatorMode.h"
#include "configuration/Observation.h"
#include "configuration/Configuration.h"

ASKAP_LOGGER(logger, ".ConfigurationFactory");

using namespace std;
using namespace askap;
using namespace askap::cp::ingest;

Configuration ConfigurationFactory::createConfiguraton(const LOFAR::ParameterSet& parset)
{
    return Configuration(
            createArrayName(parset),
            createTasks(parset),
            createAntennas(parset),
            createCorrelatorModes(parset),
            createObservation(parset),
            createMetadataTopicConfig(parset),
            createCalibrationDataServiceConfig(parset));
}

casa::String ConfigurationFactory::createArrayName(const LOFAR::ParameterSet& parset)
{
    return parset.getString("arrayname");
}

std::vector<TaskDesc> ConfigurationFactory::createTasks(const LOFAR::ParameterSet& parset)
{
    vector<TaskDesc> tasks;
    return tasks;
}

std::vector<Antenna> ConfigurationFactory::createAntennas(const LOFAR::ParameterSet& parset)
{
    vector<Antenna> antennas;
    return antennas;
}

std::map<std::string, CorrelatorMode> ConfigurationFactory::createCorrelatorModes(const LOFAR::ParameterSet& parset)
{
    map<string, CorrelatorMode> modes;

    const vector<string> modeNames = parset.getStringVector("correlator.modes");
    vector<string>::const_iterator it;
    for (it = modeNames.begin(); it != modeNames.end(); ++it) {
        const string keyBase = makeKey("correlator.mode", *it);
        const casa::uInt nChan = parset.getUint32(makeKey(keyBase, "n_chan"));
        const casa::Quantity chanWidth = asQuantity(parset.getString(
                    makeKey(keyBase, "chan_width")), "Hz");

        vector<casa::Stokes::StokesTypes> stokes;
        const vector<string> stokesStrings = parset.getStringVector(makeKey(keyBase, "stokes"));
        vector<string>::const_iterator itStokes;
        for (itStokes = stokesStrings.begin(); itStokes != stokesStrings.end(); ++itStokes) {
            stokes.push_back(casa::Stokes::type(*itStokes));
        }

        CorrelatorMode mode(*it, nChan, chanWidth, stokes);
        pair<string, CorrelatorMode> element(mode.name(), mode); 
        modes.insert(element);
    }

    return modes;
}

Observation ConfigurationFactory::createObservation(const LOFAR::ParameterSet& parset)
{
    const casa::uInt schedulingBlockID = parset.getUint32("observation.sbid");
    vector<Scan> scans;

    // Look for scans 0..*
    const unsigned int c_maxint = std::numeric_limits<unsigned int>::max();
    for (unsigned int i = 0; i < c_maxint; ++i) {
        ostringstream ss;
        ss << "observation" << "." << "scan" << i; 
        const string keyBase = ss.str();
        if (!parset.isDefined(makeKey(keyBase, "field_name"))) {
            break;
        }

        const casa::String fieldName = parset.getString(makeKey(keyBase, "field_name"));
        const casa::MDirection fieldDirection = asMDirection(
                parset.getStringVector(makeKey(keyBase, "field_direction")));
        const casa::Quantity centreFreq = asQuantity(
                parset.getString(makeKey(keyBase, "centre_freq")),
                "Hz"); // Must conform to Hz
        const casa::String correlatorMode = parset.getString(makeKey(keyBase, "correlator_mode"));

        scans.push_back(Scan(fieldName, fieldDirection, centreFreq, correlatorMode));
    }

    return Observation(schedulingBlockID, scans);
}

TopicConfig ConfigurationFactory::createMetadataTopicConfig(const LOFAR::ParameterSet& parset)
{
    const string registryHost = parset.getString("metadata_source.ice.locator_host");
    const string registryPort = parset.getString("metadata_source.ice.locator_port");
    const string topicManager = parset.getString("metadata_source.icestorm.topicmanager");
    const string topic = parset.getString("metadata_source.icestorm.topic");
    return TopicConfig(registryHost, registryPort, topicManager, topic);
}

ServiceConfig ConfigurationFactory::createCalibrationDataServiceConfig(const LOFAR::ParameterSet& parset)
{
    const string registryHost = parset.getString("cal_data_service.ice.locator_host");
    const string registryPort = parset.getString("cal_data_service.ice.locator_port");
    const string serviceName = parset.getString("cal_data_service.servicename");
    return ServiceConfig(registryHost, registryPort, serviceName);
}

std::string ConfigurationFactory::makeKey(const std::string& prefix,
                const std::string& suffix)
{
    ostringstream ss;
    ss << prefix << "." << suffix;
    return ss.str();
}

