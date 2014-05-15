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
#include <map>
#include <limits>
#include <utility>
#include <stdint.h>

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
#include "configuration/TaskDesc.h"
#include "configuration/Antenna.h"
#include "configuration/Target.h"
#include "configuration/CorrelatorMode.h"
#include "configuration/ServiceConfig.h"
#include "configuration/TopicConfig.h"

using namespace std;
using namespace askap;
using namespace askap::cp::ingest;

Configuration::Configuration(const LOFAR::ParameterSet& parset, int rank, int nprocs)
    : itsParset(parset), itsRank(rank), itsNProcs(nprocs)
{
    buildTasks();
    buildFeeds();
    buildAntennas();
    buildBaselineMap();
    buildCorrelatorModes();
    buildTargets();
}

int Configuration::rank(void) const
{
    return itsRank;
}

int Configuration::nprocs(void) const
{
    return itsNProcs;
}

casa::String Configuration::arrayName(void) const
{
    return itsParset.getString("array.name");
}

const std::vector<TaskDesc>& Configuration::tasks(void) const
{
    return itsTasks;
}

const FeedConfig& Configuration::feed(void) const
{
    if (itsFeedConfig.get() == 0) {
        ASKAPTHROW(AskapError, "Feed config not initialised");
    }
    return *itsFeedConfig;
}

const std::vector<Antenna>& Configuration::antennas(void) const
{
    return itsAntennas;
}

const BaselineMap& Configuration::bmap(void) const
{
    if (itsBaselineMap.get() == 0) {
        ASKAPTHROW(AskapError, "BaselineMap not initialised");
    }
    return *itsBaselineMap;
}

casa::uInt Configuration::schedulingBlockID(void) const
{
    return itsParset.getUint32("sb.id");
}

ServiceConfig Configuration::calibrationDataService(void) const
{
    const string registryHost = itsParset.getString("cal_data_service.ice.locator_host");
    const string registryPort = itsParset.getString("cal_data_service.ice.locator_port");
    const string serviceName = itsParset.getString("cal_data_service.servicename");
    return ServiceConfig(registryHost, registryPort, serviceName);
}

ServiceConfig Configuration::monitoringArchiverService(void) const
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

TopicConfig Configuration::metadataTopic(void) const
{
    const string registryHost = itsParset.getString("metadata_source.ice.locator_host");
    const string registryPort = itsParset.getString("metadata_source.ice.locator_port");
    const string topicManager = itsParset.getString("metadata_source.icestorm.topicmanager");
    const string topic = itsParset.getString("metadata.topic");
    return TopicConfig(registryHost, registryPort, topicManager, topic);
}

uint32_t Configuration::nScans(void) const
{
    return static_cast<uint32_t>(itsScans.size());
}

const Target& Configuration::getTargetForScan(uint32_t scanId) const
{
    if (scanId >= itsScans.size()) {
        ASKAPTHROW(AskapError, "Scan index " << scanId << " is out of range");
    }

    const string targetId = itsScans[scanId];
    map<string, Target>::const_iterator targetIter = itsTargets.find(targetId);
    if (targetIter == itsTargets.end()) {
        ASKAPTHROW(AskapError, "Target " << targetId << " not found");
    }
    return targetIter->second;
}

void Configuration::buildTasks(void)
{
    // Iterator over all tasks
    const vector<string> names = itsParset.getStringVector("tasks.tasklist");
    vector<string>::const_iterator it;
    for (it = names.begin(); it != names.end(); ++it) {
        const string keyBase = "tasks." + *it;
        const string typeStr = itsParset.getString(keyBase + ".type");
        const TaskDesc::Type type = TaskDesc::toType(typeStr);
        const LOFAR::ParameterSet params = itsParset.makeSubset(keyBase + ".params.");
        itsTasks.push_back(TaskDesc(*it, type, params));
    }
}

void Configuration::buildAntennas(void)
{
    // Build a map of name->Antenna
    const vector<string> antId = itsParset.getStringVector("antennas");
    const casa::Quantity defaultDiameter = asQuantity(itsParset.getString("antenna.ant.diameter"));
    const string defaultMount = itsParset.getString("antenna.ant.mount");
    map<string, Antenna> antennaMap;

    for (vector<string>::const_iterator it = antId.begin(); it != antId.end(); ++it) {
        const string keyBase = "antenna." + *it + ".";
        const string name = itsParset.getString(keyBase + "name");
        const vector<double> location = itsParset.getDoubleVector(keyBase + "location.itrf");

        casa::Quantity diameter;
        if (itsParset.isDefined(keyBase + "diameter")) {
            diameter = asQuantity(itsParset.getString(keyBase + "diameter"));
        } else {
            diameter = defaultDiameter;
        }

        string mount;
        if (itsParset.isDefined(keyBase + "mount")) {
            mount = itsParset.getString(keyBase + "mount");
        } else {
            mount = defaultMount;
        }

        antennaMap.insert(make_pair(name, Antenna(name, mount, location, diameter)));
    }
    
    // Now read "baselinemap.antennaidx" and build the antenna vector with the
    // ordering that maps to the baseline mapping
    const vector<string> antOrdering = itsParset.getStringVector("baselinemap.antennaidx");
    for (vector<string>::const_iterator it = antOrdering.begin(); it != antOrdering.end(); ++it) {
        map<string, Antenna>::const_iterator antit = antennaMap.find(*it);
        if (antit == antennaMap.end()) {
            ASKAPTHROW(AskapError, "Antenna " << *it << " is not configured");
        }
        itsAntennas.push_back(antit->second);
    }
}

void Configuration::buildBaselineMap(void)
{
    itsBaselineMap.reset(new BaselineMap(itsParset.makeSubset("baselinemap.")));
}

void Configuration::buildCorrelatorModes(void)
{
    const vector<string> modes = itsParset.getStringVector("correlator.modes");
    vector<string>::const_iterator it;
    for (it = modes.begin(); it != modes.end(); ++it) {
        const string name = *it;
        const string keyBase = "correlator.mode." + name + ".";
        const casa::Quantity chanWidth = asQuantity(itsParset.getString(keyBase + "chan_width"));
        const casa::uInt nChan = itsParset.getUint32(keyBase + "n_chan");

        vector<casa::Stokes::StokesTypes> stokes;
        const vector<string> stokesStrings = itsParset.getStringVector(keyBase + "stokes");
        for (vector<string>::const_iterator it = stokesStrings.begin();
                it != stokesStrings.end(); ++it) {
            stokes.push_back(casa::Stokes::type(*it));
        }

        const casa::uInt interval = itsParset.getUint32(keyBase + "interval");

        const CorrelatorMode mode(name, chanWidth, nChan, stokes, interval);
        itsCorrelatorModes.insert(make_pair(name, mode));
    }
}

void Configuration::buildTargets(void)
{
    itsScans = itsParset.getStringVector("sb.targets");
    vector<string>::const_iterator it;
    for (it = itsScans.begin(); it != itsScans.end(); ++it) {
        const string id = *it;

        // Check if this target has already been processed
        if (itsTargets.find(id) != itsTargets.end()) {
            continue;
        }

        // First time we have seen this target
        const string keyBase = "sb.target." + id + ".";
        const string name = itsParset.getString(keyBase + "field_name");
        const casa::MDirection dir = asMDirection(itsParset.getStringVector(
                    keyBase + "field_direction"));

        // Get a reference to the correlator mode
        const string modename = itsParset.getString(keyBase + "corrmode");

        map<string, CorrelatorMode>::const_iterator modeIt = itsCorrelatorModes.find(modename);
        if (modeIt == itsCorrelatorModes.end()) {
            ASKAPTHROW(AskapError, "Unknown correlator mode: " << modename);
        }

        itsTargets.insert(make_pair(id, Target(name, dir, modeIt->second)));
    }
}

void Configuration::buildFeeds(void)
{
    const uint32_t N_RECEPTORS = 2; // Only support receptors "X Y"
    const uint32_t nFeeds = itsParset.getUint32("feeds.n_feeds");
    const casa::Quantity spacing = asQuantity(itsParset.getString("feeds.spacing"), "rad");

    // Get offsets for each feed/beam
    casa::Matrix<casa::Quantity> offsets(nFeeds, N_RECEPTORS);
    for (uint32_t i = 0; i < nFeeds; ++i) {
        const string key = "feeds.feed" + utility::toString(i);
        if (!itsParset.isDefined(key)) {
            ASKAPTHROW(AskapError, "Expected " << nFeeds << " feed offsets");
        }
        const vector<casa::Double> xy = itsParset.getDoubleVector(key);
        offsets(i, 0) = spacing * xy.at(0);
        offsets(i, 1) = spacing * xy.at(1);
    }
    casa::Vector<casa::String> pols(nFeeds, "X Y");

    itsFeedConfig.reset(new FeedConfig(offsets, pols));
}
