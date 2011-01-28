/// @file QResourceManager.h
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

#ifndef ASKAP_CP_MANAGER_QRESOURCEMANAGER_H
#define ASKAP_CP_MANAGER_QRESOURCEMANAGER_H

// System includes
#include <string>

// Local package includes
#include "JobTemplate.h"
#include "IJob.h"
#include "IResourceManager.h"

namespace askap {
namespace cp {
namespace manager {

class QResourceManager : public IResourceManager {
    public:

    /// @brief Constructor.
    QResourceManager();

    /// @brief Destructor.
    virtual ~QResourceManager();

    /**
     * @return an enum containing the state of the server.
     */
    virtual ServerStatus getStatus();
    
    /**
     * Submit a new job for execution
     * @param jobTemplate   template for the job to submit.
     * @param queue     name of the queue to submit the job to.
     * @return          a object which references the submitted job.
     */
    virtual IJob::ShPtr submitJob(const JobTemplate& jobTemplate, const std::string& queue);

    private:

    std::string buildDependencyArg(JobTemplate jobTemplate);
};

};
};
};

#endif
