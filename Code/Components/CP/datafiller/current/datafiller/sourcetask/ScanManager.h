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

// Local package includes
#include "configuration/Configuration.h"

namespace askap {
namespace cp {
namespace ingest {

/// Encapsulates management of scans.
/// The ScanManager keeps track of which scan is in progress and when an
/// observation is complete.
class ScanManager {
    public:
        ScanManager(const Configuration& config);

        /// This method is called for each metadata payload received from the
        /// telescope operating system. The scan_active and scan_id fields from
        /// the metadata payload are passed in as parameters.
        ///
        /// @param[in] scanActive   the scan_active field from the TOS metadata.
        /// @param[in] scanId       the scan_id field from the TOS metadata.
        void update(const casa::Bool scanActive, const casa::String& scanId);

        /// @return true if the observation is complete, otherwise false. The
        ///         observation is deemed to be complete if the last scan has
        ///         finished.
        casa::Bool observationComplete(void) const;

        /// @return the (zero based) scan index. If the first scan has not yet
        ///         started this will return -1.
        casa::Long scanIndex(void) const;

    private:

        // A copy of the system & observation configuration
        const Configuration itsConfig;

        // Current (zero based) scan index, if the first scan has not yet
        // started this will be set to -1.
        casa::Long itsScanIndex;

        // The string (from the TOS metadata scan_id field) indicating which
        // scan is in progress. This is used to identify when a new scan has
        // been started, because a new scan_id will be present in the metadata.
        casa::String itsScanIdString;

        // Flag used to indicate the observation is complete, that is the
        // last scan has concluded.
        casa::Bool itsObsComplete;
};

}
}
}

#endif
