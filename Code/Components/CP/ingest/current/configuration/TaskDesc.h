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

/// @brief TODO: Write documentation...
class TaskDesc {
    public:

        enum Type {
            MergedSource,
            CalcUVWTask,
            ChannelAvgTask,
            CalTask,
            UVPublishTask,
            MSSink
        };

        /// @brief Constructor
        TaskDesc(const std::string& name,
                 const TaskDesc::Type type,
                 const LOFAR::ParameterSet& params);

        std::string name(void) const;

        TaskDesc::Type type(void) const;

        LOFAR::ParameterSet params(void) const;

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
