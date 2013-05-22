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
#include <sstream>
#include <vector>
#include <map>
#include <limits>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/AskapUtil.h"
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"
#include "casa/BasicSL.h"
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
#include "configuration/Observation.h"

using namespace std;
using namespace askap;
using namespace askap::cp::ingest;

Configuration::Configuration(const LOFAR::ParameterSet& parset, int rank, int ntasks)
    : itsParset(parset), itsRank(rank), itsNTasks(ntasks)
{
}

int Configuration::rank(void) const
{
    return itsRank;
}
int Configuration::ntasks(void) const
{
    return itsNTasks;
}

casa::String Configuration::arrayName(void) const
{
    return itsParset.getString("arrayname");
}

std::vector<TaskDesc> Configuration::tasks(void) const
{
    vector<TaskDesc> tasks;

    // Iterator over all tasks
    const vector<string> names = itsParset.getStringVector("tasks.tasklist");
    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        const string keyBase = makeKey("tasks", *it);
        const string typeStr = itsParset.getString(makeKey(keyBase, "type"));
        const TaskDesc::Type type = TaskDesc::toType(typeStr);
        ostringstream ss;
        ss << keyBase << ".params.";
        const LOFAR::ParameterSet params = itsParset.makeSubset(ss.str());
        tasks.push_back(TaskDesc(*it, type, params));
    }

    return tasks;
}

std::vector<Antenna> Configuration::antennas(void) const
{
    vector<Antenna> antennas;
    std::map<std::string, FeedConfig> feedConfigs = createFeeds(itsParset);

    // Iterator over all antennas
    const vector<string> names = itsParset.getStringVector("antennas.names");
    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        const string keyBase = makeKey("antennas", *it);
        const string mount = itsParset.getString(makeKey(keyBase, "mount"));
        const vector<casa::Double> location = itsParset.getDoubleVector(makeKey(keyBase, "location"));
        const casa::Quantity diameter = asQuantity(itsParset.getString(
                    makeKey(keyBase, "diameter")), "m");
        const string feedConfigName = itsParset.getString(makeKey(keyBase, "feed_config"));
        std::map<std::string, FeedConfig>::const_iterator feedIt = feedConfigs.find(feedConfigName);
        if (feedIt == feedConfigs.end()) {
                ASKAPTHROW(AskapError, "Invalid feed config: " << feedConfigName);
        }

        antennas.push_back(Antenna(*it, mount, location, diameter, feedIt->second));
    }

    return antennas;
}

BaselineMap Configuration::bmap(void) const
{
    return BaselineMap(itsParset.makeSubset("baselinemap."));
}

Observation Configuration::observation(void) const
{
    const casa::uInt schedulingBlockID = itsParset.getUint32("observation.sbid");
    vector<Scan> scans;

    // Look for scans 0..*
    const unsigned int c_maxint = std::numeric_limits<unsigned int>::max();
    for (unsigned int i = 0; i < c_maxint; ++i) {
        ostringstream ss;
        ss << "observation" << "." << "scan" << i; 
        const string keyBase = ss.str();
        if (!itsParset.isDefined(makeKey(keyBase, "field_name"))) {
            break;
        }

        const casa::String fieldName = itsParset.getString(makeKey(keyBase, "field_name"));
        const casa::MDirection fieldDirection = asMDirection(
                itsParset.getStringVector(makeKey(keyBase, "field_direction")));
        const casa::Quantity startFreq = asQuantity(
                itsParset.getString(makeKey(keyBase, "start_freq")),
                "Hz"); // Must conform to Hz

        const casa::uInt nChan = itsParset.getUint32(makeKey(keyBase, "n_chan"));
        const casa::Quantity chanWidth = asQuantity(itsParset.getString(
                    makeKey(keyBase, "chan_width")), "Hz"); // Must conform to Hz

        vector<casa::Stokes::StokesTypes> stokes;
        const vector<string> stokesStrings = itsParset.getStringVector(makeKey(keyBase, "stokes"));
        for (vector<string>::const_iterator it = stokesStrings.begin();
                it != stokesStrings.end(); ++it) {
            stokes.push_back(casa::Stokes::type(*it));
        }

        const casa::uInt interval = itsParset.getUint32(makeKey(keyBase, "interval"));

        scans.push_back(Scan(fieldName, fieldDirection, startFreq, nChan,
                    chanWidth, stokes, interval));
    }

    return Observation(schedulingBlockID, scans);
}

TopicConfig Configuration::metadataTopic(void) const
{
    const string registryHost = itsParset.getString("metadata_source.ice.locator_host");
    const string registryPort = itsParset.getString("metadata_source.ice.locator_port");
    const string topicManager = itsParset.getString("metadata_source.icestorm.topicmanager");
    const string topic = itsParset.getString("metadata_source.icestorm.topic");
    return TopicConfig(registryHost, registryPort, topicManager, topic);
}

ServiceConfig Configuration::calibrationDataService(void) const
{
    const string registryHost = itsParset.getString("cal_data_service.ice.locator_host");
    const string registryPort = itsParset.getString("cal_data_service.ice.locator_port");
    const string serviceName = itsParset.getString("cal_data_service.servicename");
    return ServiceConfig(registryHost, registryPort, serviceName);
}

ServiceConfig Configuration::MonitoringArchiverService(void) const
{
    if (itsParset.isDefined("monitoring.enabled") && itsParset.getBool("monitoring.enabled", false)) {
        const string registryHost = itsParset.getString("monitoring.ice.locator_host");
        const string registryPort = itsParset.getString("monitoring.ice.locator_port");
        const string serviceName = itsParset.getString("monitoring.servicename");
        return ServiceConfig(registryHost, registryPort, serviceName);
    } else {
        return ServiceConfig("", "", "");
    }
}

std::string Configuration::makeKey(const std::string& prefix,
                const std::string& suffix)
{
    ostringstream ss;
    ss << prefix << "." << suffix;
    return ss.str();
}

std::map<std::string, FeedConfig> Configuration::createFeeds(const LOFAR::ParameterSet& itsParset)
{
    const int nReceptors = 2; // Only support receptors "X Y"

    map<string, FeedConfig> feedConfigs;

    const vector<string> names = itsParset.getStringVector("feeds.names");
    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        const string keyBase = makeKey("feeds", *it);
        const casa::uInt nFeeds = itsParset.getUint32(makeKey(keyBase, "n_feeds"));
        const casa::Quantity spacing = asQuantity(itsParset.getString(
                    makeKey(keyBase, "spacing")), "rad");

        // Get offsets for each feed/beam
        casa::Matrix<casa::Quantity> offsets(nFeeds, nReceptors);
        for (unsigned int i = 0; i < nFeeds; ++i) {
            ostringstream ss;
            ss << keyBase << "." << "feed" << i;
            if (!itsParset.isDefined(ss.str())) {
                ASKAPTHROW(AskapError, "Expected " << nFeeds << " feed offsets");
            }
            const vector<casa::Double> xy = itsParset.getDoubleVector(ss.str());
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
