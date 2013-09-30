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
#include <string>

// ASKAPsoft includes
#include "casa/BasicSL.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "configuration/TaskDesc.h"
#include "configuration/Antenna.h"
#include "configuration/BaselineMap.h"
#include "configuration/Observation.h"
#include "configuration/TopicConfig.h"
#include "configuration/ServiceConfig.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief This class encapsulates and gives structure to the configuration
/// information passed in the Parameter Set via the Ingest Pipeline command
/// line.
class Configuration {
    public:

        /// @brief Constructor
        ///
        /// @param[in] parset   the number of tasks/processes.
        /// @param[in] rank the rank of the calling process (zero based).
        /// @param[in] ntasks the number of tasks (processes).
        Configuration(const LOFAR::ParameterSet& parset,
                      int rank = 0, int ntasks = 1);

        /// @brief Returns the rank of the calling process (zero based).
        int rank(void) const;

        /// @brief Returns the number of processes
        int nprocs(void) const;

        /// @brief The name of the array. e.g. "BETA"
        casa::String arrayName(void) const;

        /// @brief A sequence of tasks configuration.
        std::vector<TaskDesc> tasks(void) const;

        /// @brief A sequence of antennas
        std::vector<Antenna> antennas(void) const;

        /// @brief Mapping from the baseline ID that the Correlator IOC sends
        ///  and the actual antenna pair and correlation product.
        BaselineMap bmap(void) const;

        /// @brief Information about the observation itself, such as
        ///  pointing directions, etc.
        Observation observation(void) const;

        /// @brief Ice configuration for the TOS metadata topic.
        TopicConfig metadataTopic(void) const;

        /// @brief Ice configuration for the calibration data service
        ServiceConfig calibrationDataService(void) const;

        /// @brief Ice configuration for the monitoring archiver (MoniCA).
        ServiceConfig monitoringArchiverService(void) const;

    private:

        /// @brief Simple helper used to make parset keys.
        /// @param[in] prefix   a prefix string
        /// @param[in] suffix   a suffix string
        /// @returns the concatenation: prefix + "." + suffix
        static std::string makeKey(const std::string& prefix,
                                   const std::string& suffix);

        // This fnuction create a map of feed name/type to the actual feed configuration
        static std::map<std::string, FeedConfig> createFeeds(const LOFAR::ParameterSet& parset);

        // The input configuration parameter set that this "Configuration" encapsulates.
        const LOFAR::ParameterSet itsParset;

        /// The rank of this process
        const int itsRank;

        /// The total number of processes
        const int itsNProcs;
};

}
}
}

#endif
