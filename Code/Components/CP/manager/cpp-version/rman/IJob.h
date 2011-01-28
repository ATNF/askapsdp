/// @file IJob.h
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

#ifndef ASKAP_CP_MANAGER_IJOB_H
#define ASKAP_CP_MANAGER_IJOB_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"

namespace askap {
namespace cp {
namespace manager {

class IJob {
    public:

    /// Identifies the state the job is in.
    enum JobStatus {
        QUEUED,
        RUNNING,
        HELD,
        COMPLETED,
        UNKNOWN
    };
    
    /// @brief Destructor.
    virtual ~IJob();

    virtual std::string getId(void) const = 0;

    /**
     * Returns the job state.
     * @return the job state.
     */
    virtual JobStatus status(void) = 0;
    
    /**
     * @brief Abort the job. If the job is queued or held the job is simply 
     * deleted from the queue. If the job is executing it is terminated.
     */
    virtual void abort(void) = 0;

    // Shared pointer definition
    typedef boost::shared_ptr<IJob> ShPtr;

};

};
};
};

#endif
