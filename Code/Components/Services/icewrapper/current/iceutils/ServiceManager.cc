/// @file ServiceManager.cc
///
/// @copyright (c) 2014 CSIRO
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
#include "ServiceManager.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "Ice/Ice.h"

ASKAP_LOGGER(logger, ".ServiceManager");

// Using
using namespace askap;
using namespace askap::cp::icewrapper;

ServiceManager::ServiceManager(Ice::CommunicatorPtr ic, Ice::ObjectPtr obj,
        const std::string& serviceName,
        const std::string& adapterName)
    : itsComm(ic), itsObject(obj), itsServiceName(serviceName),
        itsAdapterName(adapterName)
{
}

void ServiceManager::start(void)
{
        // Create an adapter
        itsAdapter = itsComm->createObjectAdapter(itsAdapterName);
        if (!itsAdapter.get()) {
            ASKAPTHROW(AskapError, "ICE adapter initialisation failed");
        }

        // Register the service object
        itsAdapter->add(itsObject, itsComm->stringToIdentity(itsServiceName));

       // Activate the adapter
        bool activated = false;
        while (!activated) {
            const int INTERVAL = 5; // seconds
            const std::string BASE_WARN(" - will retry in " + utility::toString(INTERVAL)
                    + " seconds");
            try {
                itsAdapter->activate();
                activated = true;
            } catch (Ice::ConnectionRefusedException& e) {
                ASKAPLOG_WARN_STR(logger, "Connection refused" << BASE_WARN);
            } catch (Ice::NoEndpointException& e) {
                ASKAPLOG_WARN_STR(logger, "No endpoint" << BASE_WARN);
            } catch (Ice::NotRegisteredException& e) {
                ASKAPLOG_WARN_STR(logger, "Not registered" << BASE_WARN);
            } catch (Ice::ConnectFailedException& e) {
                ASKAPLOG_WARN_STR(logger, "Connect failed" << BASE_WARN);
            } catch (Ice::DNSException& e) {
                ASKAPLOG_WARN_STR(logger, "DNS exception" << BASE_WARN);
            } catch (Ice::SocketException& e) {
                ASKAPLOG_WARN_STR(logger, "Socket exception" << BASE_WARN);
            }
            if (!activated) {
                sleep(INTERVAL);
            }
        } 
}

void ServiceManager::waitForShutdown(void)
{
    itsComm->waitForShutdown();
}

void ServiceManager::stop(void)
{
    if (itsAdapter.get()) {
        ASKAPLOG_INFO_STR(logger, "Stopping " << itsServiceName);
        itsAdapter->deactivate();
        itsAdapter->destroy();
        ASKAPLOG_INFO_STR(logger, itsServiceName << " stopped");
    } else {
        ASKAPLOG_WARN_STR(logger, "Stop failed - " << itsServiceName
                << " not running");
    }
}
