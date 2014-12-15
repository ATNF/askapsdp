/// @file DataManager.h
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

#ifndef ASKAP_CP_INGEST_MONITORINGDATAMANAGER_H
#define ASKAP_CP_INGEST_MONITORINGDATAMANAGER_H

// System includes
#include <stdint.h>
#include <vector>
#include <string>
#include <map>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "Ice/Ice.h"
#include "iceutils/TypedValueMapper.h"

// Ice interfaces
#include "MonitoringProvider.h"

// Local package includes
#include "monitoring/MonitorPointStatus.h"
#include "configuration/Configuration.h"

namespace askap {
namespace cp {
namespace ingest {

/// An Ice/Slice monitoring point data structure
typedef askap::interfaces::monitoring::MonitorPoint IceMonitorPoint;

/// An Ice/Slice monitoring point data structure
typedef askap::interfaces::monitoring::PointStatus IcePointStatus;

/// A container for monitoring point data; decouples producer and consumer.
///
/// This class encapsulates a map data structure, allowing monitoring point
/// data to be stored by the producer, then later retrieved via the consumer.
/// This class also maps data types, allowing the producer side to deal with
/// standard C++ types, while the consumer side uses Ice types such as the
/// TypedValue map, etc.
///
/// The "point names" differ between the producer and consumer interface. Here
/// are examples of the two:
/// - Raw point name: obs.ScanId
/// - Full point name: ingest0.cp.ingest.obs.ScanId
///
/// The producer interfaces (update() and invalidatePoint()) require a raw point
/// name, that is a point name without the prefix. The consumer method (get())
/// require the full point name.
///
/// This permits the producer code to publish point data without concern for the
/// full namespace (which contains the MPI rank of the ingest process).
class DataManager {
    public:
        /// @brief Constructor.
        DataManager(const std::string& prefix);

        /// Get the monitoring points associated with the supplied pointnames.
        ///
        ///  @note If the a point name in the input vector is not present in the
        ///  set of monitoring points that particular point will be omitted from
        ///  the result set. As such, the returned vector will have length equal
        ///  to or less than the "pointnames" vector.
        ///
        ///  @param[in] pointnames  an array of points for which the monitoring data
        ///                         will be fetched.
        ///  @return    the list of monitoring points requested. However non-existent
        ///             points are omitted from the resulting set.
        std::vector<IceMonitorPoint> get(const std::vector<std::string>& pointnames);

        /// Update monitoring data for a monitoring point
        ///
        /// The prefix passed to the constructor will be added to the point
        /// name before it is stored in this object.
        ///
        /// @param[in] name     the name of the monitoring point
        /// @param[in] value    the new value for the monotoring point
        /// @param[in] status   the new status for the monitoring point
        /// @param[in] unit     a text string containing the units for
        ///                     the value. An empty string is used to
        ///                     indicate no unit.
        template <typename T>
        void update(const std::string& name,
                    const T value,
                    MonitorPointStatus_t status,
                    const std::string& unit)
        {
            updateWithIceTypes(name,
                               toIceValue(value),
                               toIceStatus(status),
                               unit);
        }

        /// Invalidate monitoring point
        ///
        /// The point name passed as the "name" parameter should be without
        /// the prefix; that is it should be the same as the point name
        /// passed to the update() method.
        ///
        /// The monitoring point specified by parameter "name" is not required
        /// to exist. If it does exist it will be "invalidated" such that calls
        /// to get() will no longer return this point.
        ///
        /// @param[in] name     the monitoring point to invalidate.
        void invalidatePoint(const std::string& name);

    private:
        /// Maps from a native "MonitorPointStatus" to an Ice "PointStatus"
        static IcePointStatus toIceStatus(const MonitorPointStatus_t status);

        /// Maps from a native type to an Ice TypedValue
        template <typename T>
        static askap::interfaces::TypedValuePtr toIceValue(const T value)
        {
            return askap::cp::icewrapper::TypedValueMapper::toTypedValue(value);
        }

        // @return Current BAT
        static long getTime(void);


        /// This method actually updates the map, holding the mutex (itsMutex)
        /// while doing so
        void updateWithIceTypes(const std::string& name,
                                askap::interfaces::TypedValuePtr value,
                                IcePointStatus status,
                                const std::string& unit);

        // The prefix that each monitoring point name will have prepended
        // to it. Eg. "cp.ingest0"
        const std::string itsPrefix;

        // Mutex to synchronise access to "itsData"
        boost::mutex itsMutex;

        // Point data table
        std::map<std::string, IceMonitorPoint> itsData;

        // No support for assignment
        DataManager& operator=(const DataManager& rhs);

        // No support for copy constructor
        DataManager(const DataManager& src);
};

}
}
}

#endif
