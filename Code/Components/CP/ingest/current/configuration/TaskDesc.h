/// @file TaskDesc.h
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

#ifndef ASKAP_CP_INGEST_TASKDESC_H
#define ASKAP_CP_INGEST_TASKDESC_H

// System includes
#include <string>

// ASKAPsoft includes
#include "Common/ParameterSet.h"

// Local package includes

namespace askap {
namespace cp {
namespace ingest {

/// @brief This class encapsulates the description of a class.
/// The class will be instantiated into the ingest pipeline based on the
/// configuration described here.
class TaskDesc {
    public:

        /// @brief An enumeration of valid task types.
        enum Type {
            MergedSource,
            NoMetadataSource,
            CalcUVWTask,
            ChannelAvgTask,
            ChannelSelTask,
            CalTask,
            MSSink,
            PhaseTrackTask,
            SimpleMonitorTask,
            ChannelFlagTask
        };

        /// @brief Constructor
        TaskDesc(const std::string& name,
                 const TaskDesc::Type type,
                 const LOFAR::ParameterSet& params);

        /// @brief A generic name for the task. This can be anything, is just a label.
        std::string name(void) const;

        /// @brief The Task type. This is the type of task that will be instantiated.
        TaskDesc::Type type(void) const;

        /// @brief A parameter subset for this specific task.
        LOFAR::ParameterSet params(void) const;

        /// @brief Maps string representations of the task type to one of the types
        /// in the "Type" enumeration.
        /// @throw AskapError   If the string could not be mapped to a known "Type".
        static TaskDesc::Type toType(const std::string& type);

    private:

        std::string itsName;
        TaskDesc::Type itsType;
        LOFAR::ParameterSet itsParams;
};

}
}
}

#endif
