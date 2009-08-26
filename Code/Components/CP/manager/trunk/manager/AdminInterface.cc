/// @file AdminInterface.cc
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
#include "AdminInterface.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"

// Local package includes

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces::component;
using LOFAR::ACC::APS::ParameterSet;

ASKAP_LOGGER(logger, ".AdminInterface");

AdminInterface::AdminInterface(const ParameterSet& parset)
    : itsParset(parset), itsState(OFFLINE)
    
{
    ASKAPLOG_INFO_STR(logger, "Creating AdminInterface");
}

AdminInterface::~AdminInterface()
{
    ASKAPLOG_INFO_STR(logger, "Destroying AdminInterface");

    // Deactivate the adapter
    itsAdapter->deactivate();
    itsAdapter->waitForDeactivate();

    // Shutdown ICE
    itsComm->shutdown();
    itsComm->waitForShutdown();
}

void AdminInterface::run(void)
{
    ASKAPLOG_INFO_STR(logger, "Running AdminInterface");

    // Initialise ICE
    itsComm = initIce(itsParset);
    ASKAPCHECK(itsComm, "Initialization of Ice communicator failed");

    itsAdapter = createAdapter(itsParset, itsComm);

    Ice::ObjectPtr object = this;
    itsAdapter->add(object, itsComm->stringToIdentity("cpmanager"));

    // Block here so main() can block on this
    itsComm->waitForShutdown();
}

Ice::CommunicatorPtr AdminInterface::initIce(const ParameterSet& parset)
{
    // Get the initialized property set.
    Ice::PropertiesPtr props = Ice::createProperties();
    ASKAPCHECK(props, "Ice properties creation failed");

    // Get (from parset) and set (into ice props) various configuration
    // parameters    
    std::string tracenet = parset.getString("ice.trace.network", "0");
    props->setProperty("Ice.Trace.Network", tracenet);

    std::string traceprot = parset.getString("ice.trace.protocol", "0");
    props->setProperty("Ice.Trace.Protocol", traceprot);

    std::string locator = parset.getString("ice.locator");
    props->setProperty("Ice.Default.Locator", locator);

    // Initialize a communicator with these properties.
    Ice::InitializationData id;
    id.properties = props;
    return Ice::initialize(id);
}

Ice::ObjectAdapterPtr AdminInterface::createAdapter(const ParameterSet& parset,
        Ice::CommunicatorPtr& ic)
{
    Ice::PropertiesPtr props = ic->getProperties();

    std::string adapterName = parset.getString("ice.adapter.name");
    std::string adapterEndpoint = parset.getString("ice.adapter.endpoints");

    // Need to create props like this (given an adapter name of TestAdapter
    // and an endpoint of tcp)
    // TestAdapter.AdapterId=TestAdapter
    // TestAdapter.Endpoints=tcp
    std::stringstream id;
    id << adapterName << "." << "AdapterId";
    std::stringstream ep;
    ep << adapterName << "." << "Endpoints";

    props->setProperty(id.str(), adapterName);
    props->setProperty(ep.str(), adapterEndpoint);

    Ice::ObjectAdapterPtr adapter = ic->createObjectAdapter(adapterName);

    ASKAPCHECK(ic, "Creation of Ice Adapter failed");

    return adapter;
}

// Ice "IComponent" interfaces
void AdminInterface::startup(const askap::interfaces::component::ParameterMap& params, const Ice::Current& cur)
{
    itsState = ONLINE;
}

void AdminInterface::shutdown(const Ice::Current& cur)
{
    itsState = OFFLINE;
}

askap::interfaces::component::ComponentTestResults AdminInterface::selfTest(const Ice::Current& cur)
{
    askap::interfaces::component::ComponentTestResults results;
    return results;
}

askap::interfaces::component::ComponentState AdminInterface::getState(const Ice::Current& cur)
{
    return itsState;
}

