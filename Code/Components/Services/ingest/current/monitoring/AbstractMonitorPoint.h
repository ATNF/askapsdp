/// @file AbstractMonitorPoint.h
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

#ifndef ASKAP_CP_INGEST_ABSTRACTMONITORPOINT_H
#define ASKAP_CP_INGEST_ABSTRACTMONITORPOINT_H

// System includes
#include <string>

// ASKAPsoft includes

// Local package includes
#include "monitoring/MonitoringSingleton.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief The base-class for all monitoring points.
/// This class exists so a generic MonitoringPoint can exist, with specialisations for
/// each data type. This class contains the basic functionality common to all
/// specialisations.
///
/// Subclasses must implement send()
template <typename T>
class AbstractMonitorPoint {
    public:
        /// Constructor
        /// @param[in] name the name of the monitoring point. For example:
        ///                 "cp.ingest0.PacketLoss"
        AbstractMonitorPoint(const std::string& name)
                : itsName(name), itsDestination(MonitoringSingleton::instance()) {
        }

        /// Destructor
        virtual ~AbstractMonitorPoint() {
        }

        /// Update the value of a monitoring point. The value will be pushed
        /// to the monitoring service.
        ///
        /// @param[in] value    the value to set the monitoring point to.
        virtual void update(const T& value) {
            if (itsDestination) send(itsName, value);
        }

        /// Sends a monitoring point with a null type. This can be used to indicate
        /// a monitoring point has no data
        virtual void updateNull(void) {
            if (itsDestination) itsDestination->sendNull(itsName, false);
        }

    protected:
        /// Subclasses implement this method
        /// @param[in] name the name of the monitoring point. e.g. "cp.ingest0.PacketLoss"
        /// @param[in] value    the value of the monitoring point
        /// @param[in] alarm    the alarm state. True if the point is in an
        ///                     alarm state, otherwise false
        virtual void send(const std::string& name, const T& value, bool alarm = false) = 0;

        /// The name of the monitoring point.
        const std::string itsName;

        /// All communication to MoniCA is via the Monitoring Singleton.
        MonitoringSingleton* itsDestination;
};

} // End namespace ingest
} // End namespace cp
} // End namespace askap

#endif
