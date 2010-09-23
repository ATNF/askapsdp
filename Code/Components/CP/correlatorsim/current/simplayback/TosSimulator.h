/// @file TosSimulator.h
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

#ifndef ASKAP_CP_TOSSIMULATOR_H
#define ASKAP_CP_TOSSIMULATOR_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "ms/MeasurementSets/MeasurementSet.h"
#include "boost/scoped_ptr.hpp"
#include "tosmetadata/MetadataOutputPort.h"

// ICE interface includes
#include "TypedValues.h"

// Local package includes
#include "simplayback/ISimulator.h"

namespace askap {
namespace cp {

/// @brief Simulates the metadata stream from the telescope operating
/// system. This metadata stream results in a metadata payload being
/// sent via an IceStorm topic for each correlator integration cycle.
class TosSimulator : public ISimulator {
    public:
        /// Constructor
        ///
        /// @param[in] dataset filename for the measurement set which
        ///                    will be used to source the metadata.
        /// @param[in] locatorHost hostname or IP address of the host
        ///                     where the Ice locator service is running.
        /// @param[in] locatorPort network port for the Ice locator service.
        /// @param[in] topicManager identify of the IceStorm topic manager
        ///                     within the Ice locator service.
        /// @param[in] topic    IceStorm topic to which the metadata will be
        ///                     published.
        TosSimulator(const std::string& dataset,
                     const std::string& locatorHost,
                     const std::string& locatorPort,
                     const std::string& topicManager,
                     const std::string& topic);

        /// Destructor
        virtual ~TosSimulator();

        /// @brief Send the next correlator integration.
        ///
        /// @return true if there are more integrations in the dataset,
        ///         otherwise false. If false is returned, sendNext()
        ///         should not be called again.
        bool sendNext(void);

    private:
        // Utility function, used to build a string out of two
        std::string makeMapKey(const std::string &prefix, const std::string &suffix);

        // Cursor (index) for the main table of the measurement set
        unsigned int itsCurrentRow;

        // Measurement set
        boost::scoped_ptr<casa::MeasurementSet> itsMS;

        // Port for output of metadata
        boost::scoped_ptr<askap::cp::icewrapper::MetadataOutputPort> itsPort;
};
};

};

#endif
