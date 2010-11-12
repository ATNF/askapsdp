/// @file JobTemplate.h
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

#ifndef ASKAP_CP_JOBTEMPLATE_H
#define ASKAP_CP_JOBTEMPLATE_H

// System includes
#include <string>
#include <map>

// Local package includes
#include "IResourceManager.h"

namespace askap {
namespace cp {
namespace manager {

/// @brief Job identifier
typedef std::string JobId;

class JobTemplate {
    public:

        /// @brief Constructor.
        JobTemplate(const std::string& name);

        /// @brief Destructor.
        ~JobTemplate();

        /// @brief Change the name of the job.
        /// @param[in] name the new name of the job.
        void setName(const std::string& name);

        /// @brief Get the name of the job.
        /// @return the name of the job.
        std::string getName(void);

        /// @brief Set the script or executable to be executed when
        /// this job runs. This should include the full path to the
        /// script/executable.
        ///
        /// @param[in] script the path and command. (e.g. /tmp/myscript.sh)
        void setScriptLocation(const std::string& script);

        /// @brief Get the pathname/commandname of the script to be
        /// executed when this job runs.
        std::string getScriptLocation(void);

        /// @brief Dependency type.
        enum DependType {
            /// Start after dependent job starts
            AFTERSTART,

            // Start after dependent job completes with no-error
            AFTEROK
        };

        /// @brief Adds dependency information to this job template.
        /// This jobs created with this template will then not start
        /// until the dependencies are fulfilled.
        ///
        /// @param[in] dependency
        /// @param[in] type
        void addDependency(JobId dependency, DependType type);

        /// @brief Remove a dependency from this job template.
        /// @param[in] dependency
        void removeDependency(JobId dependency);

        /// @brief Remove all dependencies from this job template.
        void removeAllDependencies(void);

    private:
        // The name of the job this template will create
        std::string itsName;

        // The script (including full path) which will be executed
        // when the job is executed
        std::string itsPathToScript;

        // List of jobs that any job created with this template
        // will depend on
        std::map<JobId, DependType> itsDependencies;
};

};
};
};

#endif
