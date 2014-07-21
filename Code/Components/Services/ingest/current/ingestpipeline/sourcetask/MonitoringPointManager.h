/// @file MonitoringPointManager.h
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

#ifndef ASKAP_CP_INGEST_MONITORINGPOINTMANAGER_H
#define ASKAP_CP_INGEST_MONITORINGPOINTMANAGER_H

// System includes
#include <string>

// ASKAPsoft includes
#include "cpcommon/VisChunk.h"

// Local package includes
#include "configuration/Configuration.h"
#include "monitoring/MonitorPoint.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief The primary use of this class is ensuring the monitoring points
/// are invalidated upon shutdown. The destructor for this class will send
/// null points.
class MonitoringPointManager {
    public:

        /// Constructor
        MonitoringPointManager();

        /// Destructor
        ~MonitoringPointManager();

        // Submit monitoring points to monitoring system.
        // The source data is a valid VisChunk
        void submitMonitoringPoints(const askap::cp::common::VisChunk& chunk) const;

        // Submit monitoring points to monitoring system.
        // This method just sets the specified scanid and is
        // intended to be used for scanid values of < 0 which
        // will not result in a VisCunk being created
        void submitMonitoringPoints(const int scanid) const;

        // Send null values for all managed monitoring points.
        // This essentially invalidates the monitoring point,
        // indicating the previous value is no longer valid.
        void submitNullMonitoringPoints(void) const;

        template <typename T>
        void submitPoint(const std::string& key, const T& val) const
        {
            MonitorPoint<T> point(key);
            point.update(val);
        }

    private:

        // Submits a null type. This is used to invalidate the previous value
        // in the case where the observation is complete
        void submitPointNull(const std::string& key) const;

};

}
}
}

#endif
