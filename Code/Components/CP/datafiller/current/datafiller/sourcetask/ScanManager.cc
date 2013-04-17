/// @file ScanManager.cc
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

// Include own header file first
#include "ScanManager.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "configuration/Configuration.h"


ASKAP_LOGGER(logger, ".ScanManager");

using namespace askap;
using namespace askap::cp::ingest;

ScanManager::ScanManager(const Configuration& config)
    : itsConfig(config), itsScanIndex(-1), itsObsComplete(false)
{
    const size_t nScans = itsConfig.observation().scans().size();
    if (nScans < 1) {
        ASKAPTHROW(AskapError, "Configuration contains no scans");
    }
}

void ScanManager::update(const casa::Bool scanActive, const casa::String& scanId)
{
    // 1: If the observation is complete then the scan state should no longer
    // be updated.
    if (itsObsComplete) {
        return;
    }

    // 2: Handle the case where the first usable metadata of the observation is received.
    if (itsScanIndex == -1 && scanActive) {
        itsScanIndex = 0;
        itsScanIdString = scanId;
        return;
    }

    // 3: Handle the case where the observation is in progress and the scan id changes
    if (itsScanIndex > -1 && itsScanIdString.compare(scanId) != 0) {
        if (scanActive) {
            // First handle the case where we have obviously transitioned
            // to the next scan
            itsScanIndex++;
            itsScanIdString = scanId;
            return;
        } else {
            // Next handle the case where the last scan has completed
            const size_t nScans = itsConfig.observation().scans().size();
            if ((itsScanIndex + 1) == static_cast<casa::Long>(nScans)) {
                itsObsComplete = true;
            }
        }
    }
}

casa::Bool ScanManager::observationComplete(void) const
{
    return itsObsComplete;
}

casa::Long ScanManager::scanIndex(void) const
{
    return itsScanIndex;
}
