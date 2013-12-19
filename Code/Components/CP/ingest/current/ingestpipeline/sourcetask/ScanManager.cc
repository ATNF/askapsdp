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
#include "askap/AskapUtil.h"
#include "casa/aips.h"
#include "measures/Measures/MDirection.h"
#include "casa/Quanta/MVDirection.h"
#include "casa/Quanta/MVAngle.h"
#include "configuration/Configuration.h"
#include "monitoring/MonitorPoint.h"

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

void ScanManager::update(const casa::Int scanId)
{
    // 1: If the observation is complete then the scan state should no longer
    // be updated.
    if (itsObsComplete) {
        return;
    }

    // 2: Handle the case where the first usable metadata of the observation is received.
    if (itsScanIndex == -1 && scanId >= 0) {
        ASKAPLOG_DEBUG_STR(logger, "First scan has begun - Scan Id: " << scanId);
        itsScanIndex = scanId;
        submitMonitoringPoints();
        return;
    }

    // 3: Handle the case where the observation is in progress and the scan id changes
    if (itsScanIndex >= 0 && itsScanIndex != scanId) {
        if (scanId >= 0) {
            // Handle the case where we have obviously transitioned
            // to the next scan
            ASKAPLOG_DEBUG_STR(logger, "New scan Id: " << scanId);
            itsScanIndex = scanId;
        } else {
            // Alternatively handle the case where a -1 has been received,
            // indicating end-of-observation
            itsObsComplete = true;
            const size_t nScans = itsConfig.observation().scans().size();
            if (itsScanIndex < static_cast<casa::Int>(nScans) - 1) {
                ASKAPLOG_WARN_STR(logger, "Observation ended before all specified scans were executed");
            }
        }
    }
    submitMonitoringPoints();
}

casa::Bool ScanManager::observationComplete(void) const
{
    return itsObsComplete;
}

casa::Int ScanManager::scanIndex(void) const
{
    return itsScanIndex;
}

void ScanManager::submitMonitoringPoints(void) const
{
    if (itsObsComplete) return;

    const Observation obs = itsConfig.observation();
    submitPoint<int32_t>("obs.ScanId", itsScanIndex);

    if (itsScanIndex > -1) {
        submitPoint<int32_t>("obs.nScans", obs.scans().size());
        const Scan s = obs.scans()[itsScanIndex];
        submitPoint<string>("obs.FieldName", s.name());
        submitPoint<string>("obs.dir1", askap::printLat(s.fieldDirection()));
        submitPoint<string>("obs.dir2", askap::printLon(s.fieldDirection()));
        submitPoint<string>("obs.CoordSys", casa::MDirection::showType(s.fieldDirection().type()));
        submitPoint<int32_t>("obs.Interval", s.interval() / 1000);
        submitPoint<float>("obs.StartFreq", s.startFreq().getValue("MHz"));
        submitPoint<int32_t>("obs.nChan", s.nChan());
        submitPoint<float>("obs.ChanWidth", s.chanWidth().getValue("kHz"));
    } else {
        submitPointNull("obs.nScans");
        submitPointNull("obs.FieldName");
        submitPointNull("obs.dir1");
        submitPointNull("obs.dir2");
        submitPointNull("obs.CoordSys");
        submitPointNull("obs.Interval");
        submitPointNull("obs.StartFreq");
        submitPointNull("obs.nChan");
        submitPointNull("obs.ChanWidth");
    }
}

void ScanManager::submitPointNull(const std::string& key) const
{
    MonitorPoint<int32_t> point(key);
    point.updateNull();
}
