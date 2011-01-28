/// @file QResourceManager.cc
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
#include "QResourceManager.h"

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"

// Local package includes
#include "rman/JobTemplate.h"

using namespace std;
using namespace askap::cp::manager;

ASKAP_LOGGER(logger, ".QResourceManager");

QResourceManager::QResourceManager()
{
}

QResourceManager::~QResourceManager()
{
}

IResourceManager::ServerStatus QResourceManager::getStatus()
{
    return UNCONTACTABLE;
}

IJob::ShPtr QResourceManager::submitJob(const JobTemplate& jobTemplate, const std::string& queue)
{
    return IJob::ShPtr();
}

std::string QResourceManager::buildDependencyArg(JobTemplate jobTemplate)
{
    map<std::string, JobTemplate::DependType> deps = jobTemplate.getDependencies();
    if (deps.size() == 0) {
        return "";
    }

    string str = "-W depend=";
    int count = 0;
    map<std::string, JobTemplate::DependType>::const_iterator it;
    for (it = deps.begin(); it != deps.end(); ++it) {
        if (count > 0) {
            str = str + ",";
        }
        switch (it->second) {
            case JobTemplate::AFTERSTART:
                str = str + "after:" + it->first;
                break;
            case JobTemplate::AFTEROK:
                str = str + "afterok:" + it->first;
                break;
            case JobTemplate::AFTERNOTOK:
                str = str + "afternotok:" + it->first;
                break;
            default:
                ASKAPLOG_ERROR_STR(logger, "Unhandled dependency type");
                break;
        }
        count++;
    }
    return str;
}
