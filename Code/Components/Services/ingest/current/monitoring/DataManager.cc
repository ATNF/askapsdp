/// @file DataManager.cc
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

// Include own header file first
#include "DataManager.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <vector>
#include <map>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "Ice/Ice.h"
#include "measures/Measures/MEpoch.h"
#include "casa/OS/Time.h"
#include "casa/Quanta/MVEpoch.h"

// Ice interfaces includes
#include "TypedValues.h"

// Using
using namespace askap::cp::ingest;
using std::string;
using std::vector;
using std::map;

DataManager::DataManager(const std::string& prefix)
    : itsPrefix(prefix)
{
}

std::vector<IceMonitorPoint> DataManager::get(
    const std::vector<std::string>& pointnames)
{
    vector<IceMonitorPoint> out;
    boost::mutex::scoped_lock lock(itsMutex);

    for (vector<string>::const_iterator it = pointnames.begin();
            it != pointnames.end(); ++it) {
        const map<string, IceMonitorPoint>::const_iterator element = itsData.find(*it);
        if (element != itsData.end()) {
            out.push_back(element->second);
        }
    }

    lock.unlock();
    return out;
}

void DataManager::invalidatePoint(const std::string& name)
{
    boost::mutex::scoped_lock lock(itsMutex);
    itsData.erase(itsPrefix + name);
    lock.unlock();
}

IcePointStatus DataManager::toIceStatus(const MonitorPointStatus_t status)
{
    if (status == MonitorPointStatus::INVALID) {
        return askap::interfaces::monitoring::INVALID;
    } else if (status == MonitorPointStatus::MAJORALARM) {
        return askap::interfaces::monitoring::MAJORALARM;
    } else if (status == MonitorPointStatus::MINORALARM) {
        return askap::interfaces::monitoring::MINORALARM;
    } else if (status == MonitorPointStatus::OK) {
        return askap::interfaces::monitoring::OK;
    } else {
        ASKAPTHROW(AskapError, "Unmapped PointStatus: " << status);
    }
}

long DataManager::getTime(void)
{
    casa::Time date;
    casa::MEpoch now(casa::MVEpoch(date.modifiedJulianDay()),
                     casa::MEpoch::Ref(casa::MEpoch::UTC));
    return static_cast<long>(askap::epoch2bat(now));
}

void DataManager::updateWithIceTypes(const std::string& name,
                                     askap::interfaces::TypedValuePtr value,
                                     IcePointStatus status,
                                     const std::string& unit)
{
    IceMonitorPoint point;
    point.timestamp = DataManager::getTime();
    point.name = itsPrefix + name;
    point.value = value;
    point.status = status;
    point.unit = unit;

    boost::mutex::scoped_lock lock(itsMutex);
    itsData[point.name] = point;
    lock.unlock();
}
