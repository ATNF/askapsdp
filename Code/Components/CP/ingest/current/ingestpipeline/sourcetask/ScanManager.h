/// @file ScanManager.h
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

#ifndef ASKAP_CP_INGEST_SCANMANAGER_H
#define ASKAP_CP_INGEST_SCANMANAGER_H

// System includes

// ASKAPsoft includes
#include "casa/aips.h"
#include <string>

// Local package includes
#include "configuration/Configuration.h"
#include "monitoring/MonitorPoint.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief Encapsulates management of scans.
/// @details The ScanManager keeps track of which scan is in progress and when
/// an observation is complete.
class ScanManager {
    public:

        /// Constructor
        /// @param[in] config   the configuration that defines the sequence
        ///                     of scans.
        ScanManager(const Configuration& config);

        /// Destructor
        ~ScanManager();

        /// This method is called for each metadata payload received from the
        /// telescope operating system. The scan_active and scan_id fields from
        /// the metadata payload are passed in as parameters.
        ///
        /// @param[in] scanId       the scan_id field from the TOS metadata.
        void update(const casa::Int scanId);

        /// @return true if the observation is complete, otherwise false. The
        ///         observation is deemed to be complete if the last scan has
        ///         finished.
        casa::Bool observationComplete(void) const;

        /// @return the (zero based) scan index. If the first scan has not yet
        ///         started this will return -1.
        casa::Int scanIndex(void) const;

        /// This constant is the scan id value when the TOS is not executing a
        /// scan
        static const casa::Int SCANID_IDLE = -1;

        /// This constant is the scan id when the TOS wants to signal the current
        /// scheduling block has completed execution
        static const casa::Int SCANID_OBS_COMPLETE = -2;

    private:

        // Submit monitoring points to monitoring system. This is to be
        // called when a new scan is encountered
        void submitMonitoringPoints(void) const;

        // Submits a null type. This is used to invalidate the previous value
        // in the case where the observation is complete
        void submitPointNull(const std::string& key) const;

        template <typename T>
        void submitPoint(const std::string& key, const T& val) const
        {
            MonitorPoint<T> point(key);
            point.update(val);
        }

        // A copy of the system and observation configuration
        const Configuration itsConfig;

        // Current (zero based) scan index, if the first scan has not yet
        // started this will be set to SCANID_IDLE
        casa::Int itsScanIndex;

        // Flag used to indicate the observation is complete, that is the
        // last scan has concluded.
        casa::Bool itsObsComplete;
};

}
}
}

#endif
