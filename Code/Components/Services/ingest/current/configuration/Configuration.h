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
#include <stdint.h>

// ASKAPsoft includes
#include "casa/BasicSL.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "boost/shared_ptr.hpp"
#include "configuration/TaskDesc.h"
#include "configuration/Antenna.h"
#include "configuration/BaselineMap.h"
#include "configuration/FeedConfig.h"
#include "configuration/CorrelatorMode.h"
#include "configuration/ServiceConfig.h"
#include "configuration/TopicConfig.h"
#include "configuration/MonitoringProviderConfig.h"

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
        const std::vector<TaskDesc>& tasks(void) const;

        /// @briefFeed configuration
        const FeedConfig& feed(void) const;

        /// @brief A sequence of antennas
        const std::vector<Antenna>& antennas(void) const;

        /// @brief Mapping from the baseline ID that the Correlator IOC sends
        ///  and the actual antenna pair and correlation product.
        const BaselineMap& bmap(void) const;

        /// @brief Returns the correlator configuration for a
        /// given correlator mode name
        const CorrelatorMode& lookupCorrelatorMode(const std::string& modename) const;

        // Returns the scheduling block id for this observation
        casa::uInt schedulingBlockID(void) const;

        /// @brief Ice configuration for the TOS metadata topic.
        TopicConfig metadataTopic(void) const;

        /// @brief Ice configuration for the calibration data service
        ServiceConfig calibrationDataService(void) const;

        /// @brief Ice configuration for the monitoring provider interface
        MonitoringProviderConfig monitoringConfig(void) const;

    private:

        void buildTasks(void);

        void buildFeeds(void);

        void buildAntennas(void);

        void buildBaselineMap(void);

        void buildCorrelatorModes(void);

        // The input configuration parameter set that this "Configuration" encapsulates.
        const LOFAR::ParameterSet itsParset;

        /// The rank of this process
        const int itsRank;

        /// The total number of processes
        const int itsNProcs;

        boost::shared_ptr<FeedConfig> itsFeedConfig;

        std::vector<Antenna> itsAntennas;

        std::vector<TaskDesc> itsTasks;

        std::map<std::string, CorrelatorMode> itsCorrelatorModes;

        boost::shared_ptr<BaselineMap> itsBaselineMap;
};

}
}
}

#endif
