/// @file MonitoringSingleton.cc
///
/// @copyright (c) 2013-2014 CSIRO
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
#include "MonitoringSingleton.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "Ice/Ice.h"
#include "iceutils/CommunicatorConfig.h"
#include "iceutils/CommunicatorFactory.h"

// Ice interfaces includes
#include "TypedValues.h"

// Local package includes
#include "configuration/Configuration.h"
#include "monitoring/DataManager.h"
#include "monitoring/MonitoringProviderImpl.h"

ASKAP_LOGGER(logger, ".MonitoringSingleton");

// Using
using namespace askap::cp::ingest;
using askap::cp::icewrapper::ServiceManager;
using askap::cp::icewrapper::CommunicatorConfig;
using askap::cp::icewrapper::CommunicatorFactory;

// Initialise statics
boost::scoped_ptr<DataManager> MonitoringSingleton::theirDataManager;
boost::scoped_ptr<ServiceManager> MonitoringSingleton::theirServiceManager;
Ice::CommunicatorPtr MonitoringSingleton::theirComm;

MonitoringSingleton::MonitoringSingleton()
{
}

MonitoringSingleton::~MonitoringSingleton()
{
}

void MonitoringSingleton::init(const Configuration& config)
{
    const MonitoringProviderConfig monconf = config.monitoringConfig();
    if (monconf.registryHost().empty()) return;

    if (!theirDataManager.get() && !theirServiceManager.get()) {
        // Configure the data manager, the repository for current monitoring
        // point data
        const string prefix = "ingest" + utility::toString(config.rank()) + ".cp.ingest.";
        theirDataManager.reset(new DataManager(prefix));

        // Configure the Ice communicator
        CommunicatorConfig cc(monconf.registryHost(), monconf.registryPort());
        cc.setAdapter(monconf.adapterName(), "tcp", true);
        CommunicatorFactory commFactory;
        theirComm = commFactory.createCommunicator(cc);

        // Create the object which implements the Monitoring Provider Service
        Ice::ObjectPtr obj = new MonitoringProviderImpl(*theirDataManager);

        // Create the service manager and start the service running
        theirServiceManager.reset(new ServiceManager(theirComm, obj,
                                  monconf.serviceIdentity(), monconf.adapterName()));
        theirServiceManager->start();
    } else {
        ASKAPTHROW(AskapError, "Monitoring Singleton already initialised");
    }
}

void MonitoringSingleton::destroy()
{
    if (theirServiceManager.get()) {
        theirServiceManager->stop();
        theirServiceManager.reset();
    }

    theirComm->destroy();
    theirDataManager.reset();
}

void MonitoringSingleton::invalidatePoint(const std::string& name)
{
    if (theirDataManager.get()) {
        theirDataManager->invalidatePoint(name);
    }
}
