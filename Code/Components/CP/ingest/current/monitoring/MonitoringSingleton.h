/// @file MonitoringSingleton.h
///
/// @copyright (c) 2012 CSIRO
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

#ifndef ASKAP_CP_INGEST_MONITORINGSINGLETON_H
#define ASKAP_CP_INGEST_MONITORINGSINGLETON_H

// System includes
#include <stdint.h>
#include <string>
#include <deque>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"
#include "Ice/Ice.h"
#include "MoniCA.h" // ICE generated interface

// Local package includes
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {

class MonitoringSingleton {
    public:
        /// @brief Obtain the singleton instance of the monitoring
        /// data interface singleton.
        ///
        /// @return the singleton instance.
        static MonitoringSingleton* instance(void);

        /// Initialise the singleton instance
        static void init(const Configuration& config);

        /// Destroy the singleton instance
        static void destroy();

        /// @brief Destructor.
        ~MonitoringSingleton();

        void sendBool(const std::string& name, bool value);
        void sendFloat(const std::string& name, float value);
        void sendDouble(const std::string& name, double value);
        void sendInt32(const std::string& name, int32_t value);
        void sendInt64(const std::string& name, int64_t value);
        void sendString(const std::string& name, const std::string& value);

    private:

        /// @brief Constructor.
        MonitoringSingleton(const Configuration& config);

        // Adds a monitoring point update to the queue to be sent to MoniCA
        void enqueue(const std::string& name, atnf::atoms::mon::comms::DataValuePtr value);

        // Returns time since the epoch
        long getTime(void) const;

        // Entry method for sender thread
        void senderrun(void);

        // Singleton instance of this calss
        static MonitoringSingleton* itsInstance;

        // Configuration data
        const Configuration itsConfig;

        // Ice communicator
        Ice::CommunicatorPtr itsComm;

        // Proxy object for MoniCA service
        atnf::atoms::mon::comms::MoniCAIcePrx itsMonicaProxy;

        // Buffer to act as a mailbox between the caller and the sender thread
        std::deque<atnf::atoms::mon::comms::PointDataIce> itsBuffer;

        // Mutex to synchronise access to "itsBuffer"
        boost::mutex itsMutex;

        // Condition variable user for synchronisation between the thread that enqueues
        // monitoring point updates, and the one that sends them
        boost::condition itsCondVar;

        // Thread for sending data to MoniCA
        boost::shared_ptr<boost::thread> itsThread;

        // The prefix that each monitoring point name will have prepended
        // to it. Eg. "cp.ingest0"
        std::string itsPrefix;

        // No support for assignment
        MonitoringSingleton& operator=(const MonitoringSingleton& rhs);

        // No support for copy constructor
        MonitoringSingleton(const MonitoringSingleton& src);
};

}
}
}

#endif
