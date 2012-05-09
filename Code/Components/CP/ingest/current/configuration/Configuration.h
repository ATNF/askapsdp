/// @file Configuration.h
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

#ifndef ASKAP_CP_INGEST_CONFIGURATION_H
#define ASKAP_CP_INGEST_CONFIGURATION_H

// System includes
#include <vector>

// ASKAPsoft includes
#include "casa/BasicSL.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "configuration/TaskDesc.h"
#include "configuration/Antenna.h"
#include "configuration/Observation.h"
#include "configuration/TopicConfig.h"
#include "configuration/ServiceConfig.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief TODO: Write documentation...
class Configuration {
    public:

        /// @brief Constructor
        Configuration(const LOFAR::ParameterSet& parset);

        casa::String arrayName(void) const;
        std::vector<TaskDesc> tasks(void) const;
        std::vector<Antenna> antennas(void) const;
        Observation observation(void) const;
        TopicConfig metadataTopic(void) const;
        ServiceConfig calibrationDataService(void) const;

    private:
        static std::string makeKey(const std::string& prefix,
                const std::string& suffix);

        static std::map<std::string, FeedConfig> createFeeds(const LOFAR::ParameterSet& parset);
        const LOFAR::ParameterSet itsParset;
};

}
}
}

#endif
