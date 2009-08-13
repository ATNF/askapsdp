/// @file Workflow.cc
///
/// @copyright (c) 2009 CSIRO
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
#include "Workflow.h"

// System includes
#include <string>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"

// Local package includes
#include "activities/Activity.h"
#include "activities/ActivityFactory.h"

// Using
using namespace askap;
using namespace askap::cp;
using LOFAR::ACC::APS::ParameterSet;

ASKAP_LOGGER(logger, ".Workflow");

Workflow::Workflow(const Ice::CommunicatorPtr& ic,
        const Ice::ObjectAdapterPtr& adapter,
        const ParameterSet& parset,
        const std::string& runtimeName)
    : itsComm(ic), itsAdapter(adapter), itsParset(parset),
    itsRuntimeName(runtimeName)
{
    ASKAPLOG_INFO_STR(logger, "Creating workflow");
    itsDesc = parse();
    createAll(itsDesc);
}

Workflow::~Workflow()
{
    ASKAPLOG_INFO_STR(logger, "Destroying workflow");
    while (itsActivities.size() != 0) {
        itsActivities.back().reset();
        itsActivities.pop_back();
    }
}

void Workflow::start(void)
{
    ASKAPLOG_INFO_STR(logger, "Starting workflow");
    attachAll();
    startAll();
}

void Workflow::stop(void)
{
    ASKAPLOG_INFO_STR(logger, "Stopping workflow");
    stopAll();
    detachAll();
}

// Parse workflow descriptor file
//
// Example activity descriptor:
// askap.cp.frontend.workflow.activity0.runtime    =   cpfe_runtime1
// askap.cp.frontend.workflow.activity0.type       =   AddMetadata
// askap.cp.frontend.workflow.activity0.name       =   AddMetadata-cb0
// askap.cp.frontend.workflow.activity0.in.port0   =   MetadataStream0
// askap.cp.frontend.workflow.activity0.in.port1   =   VisStream0
// askap.cp.frontend.workflow.activity0.out.port0  =   AnnotatedVisStream0
// askap.cp.frontend.workflow.activity0.custom.blah  = Hello
std::vector<askap::cp::ActivityDesc> Workflow::parse(void)
{
    ASKAPLOG_INFO_STR(logger, "Parsing workflow");
    const unsigned int ACTIVITY_MAX = 65535;
    const unsigned int PORTS_MAX = 65535;
    std::vector<askap::cp::ActivityDesc> list;

    for (unsigned int i = 0; i < ACTIVITY_MAX; ++i) {
        std::stringstream ss;
        ss << "activity" << i << ".";
        ASKAPLOG_INFO_STR(logger, "Looking for " << ss.str());
        ParameterSet subset = itsParset.makeSubset(ss.str());
        if (subset.size() == 0) {
            break;
        }

        ASKAPLOG_INFO_STR(logger, "Found " << ss.str());

        // Add the simple types
        const std::string runtime = subset.getString("runtime");
        if (runtime != itsRuntimeName) {
            // This activity is not to be deployed on this runtime instance
            continue;
        }
        const std::string type = subset.getString("type");
        const std::string name = subset.getString("name");

        ActivityDesc desc;
        desc.setRuntime(runtime);
        desc.setType(type);
        desc.setName(name);

        // Make another subset for the custom parameters
        ParameterSet custom = subset.makeSubset("custom.");
        desc.setParset(custom);

        // Process input ports
        for (unsigned int ip; ip < PORTS_MAX; ++ip) {
            std::stringstream iss;
            iss << "in.port" << ip;
            const std::string stream = subset.getString(iss.str(), "");
            if (stream == "") {
                break;
            }
            desc.addInPortMapping(stream);
        }

        // Process output ports
        for (unsigned int op; op < PORTS_MAX; ++op) {
            std::stringstream oss;
            oss << "out.port" << op;
            const std::string stream = subset.getString(oss.str(), "");
            if (stream == "") {
                break;
            }
            desc.addOutPortMapping(stream);
        }

        list.push_back(desc);
    }

    return list; 
}

// Create all activities
void Workflow::createAll(const std::vector<ActivityDesc>& activities)
{
    ASKAPLOG_INFO_STR(logger, "CreateAll() in workflow. Num activities = " << activities.size());
    ActivityFactory factory(itsComm, itsAdapter);

    std::vector<ActivityDesc>::const_iterator it;
    for (it = activities.begin(); it != activities.end(); ++it) {
        ASKAPLOG_INFO_STR(logger, "Creating activity " << it->getName()
                << " of type " << it->getType());
        Activity::ShPtr activity = factory.makeActivity(it->getType());
        activity->setName(it->getName());

        itsActivities.push_back(activity);
    }
}

// Attach all activities to streams
void Workflow::attachAll(void)
{
}

// Detach all activities from streams
void Workflow::detachAll(void)
{
}

// Start run thread on all activities
void Workflow::startAll(void)
{
    std::vector<Activity::ShPtr>::iterator it;
    for (it = itsActivities.begin(); it != itsActivities.end(); ++it) {
        (*it)->start();
    }
}

// Stop run thread on all activities
void Workflow::stopAll(void)
{
    std::vector<Activity::ShPtr>::iterator it;
    for (it = itsActivities.begin(); it != itsActivities.end(); ++it) {
        (*it)->stop();
    }
}
