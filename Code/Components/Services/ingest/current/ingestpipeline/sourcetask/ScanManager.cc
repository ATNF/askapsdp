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

ASKAP_LOGGER(logger, ".ScanManager");

using namespace askap;
using namespace askap::cp::ingest;

ScanManager::ScanManager()
    : itsScanIndex(SCANID_IDLE), itsObsComplete(false)
{
}

ScanManager::~ScanManager()
{
}

void ScanManager::update(const casa::Int newScanId)
{
    // 1: If the observation is complete then the scan state should no longer
    // be updated.
    if (itsObsComplete) {
        return;
    }

    // 2: Handle the end-of-observation (scanid of -2)
    if (newScanId == SCANID_OBS_COMPLETE) {
        itsObsComplete = true;
        itsScanIndex = newScanId;
        return;
    }

    // 3: Handle the "IDLE" scan (scanid of -1)
    if (newScanId == SCANID_IDLE) {
        itsScanIndex = newScanId;
        return;
    }

    // 4: Handle the case where we start a new (real) scan.
    // i.e. scanId is >= 0
    if (newScanId >= 0 && itsScanIndex != newScanId) {
        ASKAPLOG_DEBUG_STR(logger, "New scan Id: " << newScanId);
        itsScanIndex = newScanId;
        return;
    }

    // 5: Handle an unknown negative scan id
    if (newScanId < 0 && newScanId != SCANID_OBS_COMPLETE
            && newScanId != SCANID_IDLE) {
        ASKAPTHROW(AskapError, "Unexpected scan id " << newScanId);
    }
}

casa::Bool ScanManager::observationComplete(void) const
{
    return itsObsComplete;
}

casa::Int ScanManager::scanIndex(void) const
{
    return itsScanIndex;
}
