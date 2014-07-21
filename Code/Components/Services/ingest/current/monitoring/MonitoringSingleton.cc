/// @file MonitoringSingleton.cc
///
/// @copyright (c) 2013 CSIRO
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
#include <vector>
#include <ctime>
#include <exception>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "boost/shared_ptr.hpp"
#include "Ice/Ice.h"
#include "iceutils/CommunicatorConfig.h"
#include "iceutils/CommunicatorFactory.h"
#include "measures/Measures/MEpoch.h"
#include "casa/OS/Time.h"
#include "casa/Quanta/MVEpoch.h"
#include "MoniCA.h" // ICE generated interface

// Local package includes
#include "configuration/Configuration.h" // Includes all configuration attributes too

ASKAP_LOGGER(logger, ".MonitoringSingleton");

using namespace askap;
using namespace askap::cp::icewrapper;
using namespace askap::cp::ingest;
using namespace atnf::atoms::mon::comms;

// Initialise statics
MonitoringSingleton* MonitoringSingleton::itsInstance = 0;

MonitoringSingleton::MonitoringSingleton(const Configuration& config)
        : itsConfig(config)
{
    // Create the prefix for all point names
    itsPrefix = "ingest" + utility::toString(config.rank()) + ".cp.ingest.";

    // Setup Ice and try to connect to the MoniCA service
    tryConnect();

    // Start the sender thread
    itsThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&MonitoringSingleton::senderrun, this)));
}

MonitoringSingleton::~MonitoringSingleton()
{
    if (itsThread.get()) {
        itsThread->interrupt();
        itsThread->join();
    }

    if (itsComm) {
        itsComm->destroy();
    }
}

MonitoringSingleton* MonitoringSingleton::instance(void)
{
    return itsInstance;
}

void MonitoringSingleton::init(const Configuration& config)
{
    if (!itsInstance) {
        itsInstance = new MonitoringSingleton(config);
    } else {
        ASKAPTHROW(AskapError, "Monitoring Singleton already initialised");
    }
}

void MonitoringSingleton::destroy()
{
    if (itsInstance) {
        delete itsInstance;
        itsInstance = 0;
    }
}

void MonitoringSingleton::sendBool(const std::string& name, bool value, bool alarm)
{
    enqueue(name, new DataValueBoolean(DTBoolean, value), alarm);
}

void MonitoringSingleton::sendFloat(const std::string& name, float value, bool alarm)
{
    enqueue(name, new DataValueFloat(DTFloat, value), alarm);
}

void MonitoringSingleton::sendDouble(const std::string& name, double value, bool alarm)
{
    enqueue(name, new DataValueDouble(DTDouble, value), alarm);
}

void MonitoringSingleton::sendInt32(const std::string& name, int32_t value, bool alarm)
{
    enqueue(name, new DataValueInt(DTInt, value), alarm);
}

void MonitoringSingleton::sendInt64(const std::string& name, int64_t value, bool alarm)
{
    enqueue(name, new DataValueLong(DTLong, value), alarm);
}

void MonitoringSingleton::sendString(const std::string& name, const std::string& value, bool alarm)
{
    enqueue(name, new DataValueString(DTString, value), alarm);
}

void MonitoringSingleton::sendNull(const std::string& name, bool alarm)
{
    enqueue(name, new DataValue(DTNull), alarm);
}

void MonitoringSingleton::enqueue(const std::string& name,
                                  atnf::atoms::mon::comms::DataValuePtr value, bool alarm)
{
    // Add monitoring point update to the front of the queue
    PointDataIce pd;
    pd.name = itsPrefix + name;
    pd.timestamp = getTime();
    pd.alarm = alarm;
    pd.value = value;
    boost::mutex::scoped_lock lock(itsMutex);
    itsBuffer.push_front(pd);

    //  If the queue is too large, discard one from the end of the queue
    //  (i.e.) discard older data
    if (itsBuffer.size() > 1000) {
        itsBuffer.pop_back();
    }

    // Notify any waiters
    lock.unlock();
    itsCondVar.notify_all();
}

long MonitoringSingleton::getTime(void) const
{
    casa::Time date;
    casa::MEpoch now(casa::MVEpoch(date.modifiedJulianDay()),
            casa::MEpoch::Ref(casa::MEpoch::UTC));
    return static_cast<long>(askap::epoch2bat(now));
}

void MonitoringSingleton::senderrun(void)
{
    vector<string> names;
    vector<PointDataIce> values;

    try {
        while (!(boost::this_thread::interruption_requested())) {
            // Check that the connection to Monica service has been made
            if (!itsMonicaProxy) {
                const bool success = tryConnect();

                if (!success) {
                    // Throttle the retry rate
                    boost::this_thread::sleep(boost::posix_time::seconds(60));
                    continue;
                }
            }

            // Wait for some data to send
            boost::mutex::scoped_lock lock(itsMutex);

            while (itsBuffer.empty()) {
                itsCondVar.wait(lock);
            }

            // Extract the data from the buffer
            while (!itsBuffer.empty()) {
                names.push_back(itsBuffer.back().name);
                values.push_back(itsBuffer.back());
                itsBuffer.pop_back();
                boost::this_thread::interruption_point();
            }

            // Send the batch
            try {
                itsMonicaProxy->setData(names, values, "0000", "0000");
            } catch (Ice::Exception& e) {
                ASKAPLOG_DEBUG_STR(logger, "Ice::Exception: " << e);
            } catch (std::exception& e) {
                ASKAPLOG_DEBUG_STR(logger, "Unexpected exception");
            }

            names.clear();
            values.clear();
        }
    } catch (boost::thread_interrupted& e) {
        // Nothing to do, just return
    }
}

bool MonitoringSingleton::tryConnect(void)
{
    try {
        // Setup ICE
        if (!itsComm) {
            const string registryHost = itsConfig.monitoringArchiverService().registryHost();

            if (registryHost.empty()) return false;

            const string registryPort = itsConfig.monitoringArchiverService().registryPort();
            CommunicatorConfig commconfig(registryHost, registryPort);
            CommunicatorFactory commFactory;
            itsComm = commFactory.createCommunicator(commconfig);

            if (!itsComm) return false;
        }

        if (!itsMonicaProxy) {
            const string serviceName = itsConfig.monitoringArchiverService().serviceIdentity();
            Ice::ObjectPrx base = itsComm->stringToProxy(serviceName);
            itsMonicaProxy = atnf::atoms::mon::comms::MoniCAIcePrx::checkedCast(base);

            if (!itsMonicaProxy) return false;
        }
    } catch (Ice::ConnectionRefusedException& e) {
        ASKAPLOG_WARN_STR(logger, "Connection refused exception: " << e);
        return false;
    } catch (Ice::NoEndpointException& e) {
        ASKAPLOG_WARN_STR(logger, "No endpoint exception: " << e);
        return false;
    } catch (Ice::NotRegisteredException& e) {
        ASKAPLOG_WARN_STR(logger, "Not registered exception: " << e);
        return false;
    } catch (Ice::ConnectFailedException& e) {
        ASKAPLOG_WARN_STR(logger, "Connection failed exception: " << e);
        return false;
    } catch (Ice::DNSException& e) {
        ASKAPLOG_WARN_STR(logger, "DNS exception: " << e);
        return false;
    } catch (std::exception& e) {
        ASKAPLOG_WARN_STR(logger, "Unexpected exception");
        return false;
    }

    return true;
}
