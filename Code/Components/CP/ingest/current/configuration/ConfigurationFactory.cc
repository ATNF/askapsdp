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
#include "Common/ParameterSet.h"
#include "casa/aips.h"
#include "casa/BasicSL.h"
#include "casa/Quanta.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"

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

    // Iterator over all tasks
    const vector<string> names = parset.getStringVector("tasks.tasklist");
    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        const string keyBase = makeKey("tasks", *it);
        const string typeStr = parset.getString(makeKey(keyBase, "type"));
        const TaskDesc::Type type = TaskDesc::toType(typeStr);
        ostringstream ss;
        ss << keyBase << ".params.";
        const LOFAR::ParameterSet params = parset.makeSubset(ss.str());

        tasks.push_back(TaskDesc(*it, type, params));
    }

    return tasks;
}

std::vector<Antenna> ConfigurationFactory::createAntennas(const LOFAR::ParameterSet& parset)
{
    vector<Antenna> antennas;
    std::map<std::string, FeedConfig> feedConfigs = createFeeds(parset);

    // Iterator over all antennas
    const vector<string> names = parset.getStringVector("antennas.names");
    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        const string keyBase = makeKey("antennas", *it);
        const string mount = parset.getString(makeKey(keyBase, "mount"));
        const vector<casa::Double> location = parset.getDoubleVector(makeKey(keyBase, "location"));
        const casa::Quantity diameter = asQuantity(parset.getString(
                    makeKey(keyBase, "diameter")), "m");
        const string feedConfigName = parset.getString(makeKey(keyBase, "feed_config"));
        std::map<std::string, FeedConfig>::const_iterator feedIt = feedConfigs.find(feedConfigName);
        if (feedIt == feedConfigs.end()) {
                ASKAPTHROW(AskapError, "Invalid feed config: " << feedConfigName);
        }

        antennas.push_back(Antenna(*it, mount, location, diameter, feedIt->second));
    }

    return antennas;
}

std::map<std::string, CorrelatorMode> ConfigurationFactory::createCorrelatorModes(const LOFAR::ParameterSet& parset)
{
    map<string, CorrelatorMode> modes;

    const vector<string> names = parset.getStringVector("correlator.modes");
    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
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

std::map<std::string, FeedConfig> ConfigurationFactory::createFeeds(const LOFAR::ParameterSet& parset)
{
    const int nReceptors = 2; // Only support receptors "X Y"

    map<string, FeedConfig> feedConfigs;

    const vector<string> names = parset.getStringVector("feeds.names");
    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        const string keyBase = makeKey("feeds", *it);
        const casa::uInt nFeeds = parset.getUint32(makeKey(keyBase, "n_feeds"));
        const casa::Quantity spacing = asQuantity(parset.getString(
                    makeKey(keyBase, "spacing")), "rad");

        // Get offsets for each feed/beam
        casa::Matrix<casa::Quantity> offsets(nFeeds, nReceptors);
        for (unsigned int i = 0; i < nFeeds; ++i) {
            ostringstream ss;
            ss << keyBase << "." << "feed" << i;
            if (!parset.isDefined(ss.str())) {
                ASKAPTHROW(AskapError, "Expected " << nFeeds << " feed offsets");
            }
            const vector<casa::Double> xy = parset.getDoubleVector(ss.str());
            offsets(i, 0) = spacing * xy.at(0);
            offsets(i, 1) = spacing * xy.at(1);
        }

        casa::Vector<casa::String> pols(nFeeds, "X Y");

        // Create the FeedConfig instance and add to the map
        pair<string, FeedConfig> element(*it, FeedConfig(offsets, pols)); 
        feedConfigs.insert(element);
    }

    return feedConfigs;
}
