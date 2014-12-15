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
#include <string>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "Ice/Ice.h"
#include "iceutils/ServiceManager.h"

// Ice interfaces
#include "MonitoringProvider.h"

// Local package includes
#include "monitoring/MonitorPointStatus.h"
#include "monitoring/DataManager.h"
#include "configuration/Configuration.h"

namespace askap {
namespace cp {
namespace ingest {

class MonitoringSingleton {
    public:
        /// @brief Destructor.
        ~MonitoringSingleton();

        /// Initialise the singleton instance
        static void init(const Configuration& config);

        /// Destroy the singleton instance
        /// This method can be called safely even if init() has not
        /// been called, in which case this method will return without
        /// action.
        static void destroy();

        ///  Submit an update to a monitoring point.
        ///
        /// If a value for this point is already set it will be replaced with
        ///  the supplied data.
        ///
        ///  This method adds a "cp.ingest." prefix to all monitoring points.
        ///
        /// @param[in] name     a name identifying the monitoring point.
        /// @param[in] value    the value a point has (e.g. some measurement or state)
        /// @param[in] status   the status of the point
        /// @param[in] unit     unit associated with the value
        template <typename T>
        static void update(const std::string& name, const T value,
                           const MonitorPointStatus_t status,
                           const std::string& unit)
        {
            if (theirDataManager.get()) {
                theirDataManager->update(name,
                                         value,
                                         status, unit);
            }
        }

        /// Submit an update to a monitoring point (without a unit)
        template <typename T>
        static void update(const std::string& name, const T value,
                           const MonitorPointStatus_t status)
        {
            update(name, value, status, "");
        }

        /// Updates a monitoring point to a state indicating the point is
        /// invalid.
        ///
        /// @param[in] name     a name identifying the monitoring point.
        static void invalidatePoint(const std::string& name);

    private:
        // Instance of the Data Manager
        static boost::scoped_ptr<DataManager> theirDataManager;

        // Instance of the Service Manager
        static boost::scoped_ptr<askap::cp::icewrapper::ServiceManager> theirServiceManager;

        // Ice communicator
        static Ice::CommunicatorPtr theirComm;

        /// @brief Constructor.
        MonitoringSingleton();

        // No support for assignment
        MonitoringSingleton& operator=(const MonitoringSingleton& rhs);

        // No support for copy constructor
        MonitoringSingleton(const MonitoringSingleton& src);
};

}
}
}

#endif
