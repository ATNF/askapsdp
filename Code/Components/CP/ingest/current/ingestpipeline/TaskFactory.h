/// @file TaskFactory.h
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

#ifndef ASKAP_CP_INGEST_TASKFACTORY_H
#define ASKAP_CP_INGEST_TASKFACTORY_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/sourcetask/ISource.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {

class TaskFactory {
    public:

        TaskFactory(const Configuration& config);

        /// @brief Creates an instance of a task given the parameters specified
        /// in the parameter set.
        ///
        /// The parameter set should have a single "type" element, and one or 
        /// more "params" elements. For example the following defines an
        /// instance of the calibration applicator task:
        ///
        /// type              = CalTask
        /// params.gain.g11.0 = [1.0]
        /// params.gain.g11.1 = [1.0]
        /// params.gain.g11.2 = [1.0]
        /// ..
        /// ..
        ///
        /// @param[in] taskDescription   an object describing the task.
        /// @return a shared pointer to a task.
        /// @throw AskapError   if the task type is unknown.
        ITask::ShPtr createTask(const TaskDesc& taskDescription);

        /// @brief Create a source given the parameters specified
        /// in the parameter set.
        ///
        /// TODO: It would be good to just treat a souce as a normal task.
        ///
        /// @return a shared pointer to the source task.
        boost::shared_ptr< ISource > createMergedSource(void);

        /// @brief Create a source given the parameters specified
        /// in the parameter set.
        boost::shared_ptr< ISource > createNoMetadataSource(void);

    private:
        const Configuration itsConfig;
};

}
}
}

#endif
