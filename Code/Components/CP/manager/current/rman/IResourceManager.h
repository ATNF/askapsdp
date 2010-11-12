/// @file IResourceManager.h
///
/// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_CP_IRESOURCEMANAGER_H
#define ASKAP_CP_IRESOURCEMANAGER_H

// System includes
#include <string>

// Local package includes
#include "JobTemplate.h"

namespace askap {
namespace cp {
namespace manager {

// Forward declarations
class JobTemplate;

/// @brief Job identifier
typedef std::string JobId;

class IResourceManager {
    public:
        /// @brief Destructor.
        virtual ~IResourceManager();

        ///////////////////////////////
        // Server Management
        ///////////////////////////////
        
        enum ServerStatus
        {
            AVAILABLE,
            UNCONTACTABLE
        };

        virtual ServerStatus serverStatus(void) = 0;

        ///////////////////////////////
        // Job Management
        ///////////////////////////////

        /// @brief Submit a new job for execution
        virtual JobId submitJob(JobTemplate jobTemplate, const std::string queue) = 0;

        /// @brief Delete a job.
        /// If the job is queued or held the job is simply deleted from the
        /// queue. If the job is executing it is terminated.
        virtual void deleteJob(const JobId& job) = 0;

        /// @brief Query the status of a job.
        virtual JobId getJobState(void) = 0;

};

};
};
};

#endif
