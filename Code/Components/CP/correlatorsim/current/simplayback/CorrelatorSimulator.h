/// @file CorrelatorSimulator.h
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

#ifndef ASKAP_CP_SIMPLAYBACK_CORRELATORSIMULATOR_H
#define ASKAP_CP_SIMPLAYBACK_CORRELATORSIMULATOR_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "ms/MeasurementSets/MeasurementSet.h"
#include "boost/scoped_ptr.hpp"

// Local package includes
#include "simplayback/ISimulator.h"
#include "simplayback/VisPort.h"
#include "simplayback/BaselineMap.h"

namespace askap {
namespace cp {

/// @brief Simulates the visibility stream from the correlator.
class CorrelatorSimulator : public ISimulator {
    public:
        /// Constructor
        ///
        /// @param[in] dataset  filename for the measurement set which will be
        ///                     used to source the visibilities.
        /// @param[in] hostname hostname or IP address of the host to which the
        ///                     UDP data stream will be sent.
        /// @param[in] port     UDP port number to which the UDP data stream will
        ///                     be sent.
        /// @param[in] expansionFactor  the channel multiplication factor. A
        ///     non-unity expansion factor allows a small input dataset to be
        ///     used to produce a larger output data stream. For example
        ///     simulating a small 304 channel (1MHz channels) dataset and using
        ///     an expansion factor of 54 to get to a 16416 (18.5KHz channels)
        ///     data stream.
        CorrelatorSimulator(const std::string& dataset,
                            const std::string& hostname,
                            const std::string& port,
                            const BaselineMap& bmap,
                            const unsigned int expansionFactor = 1);

        /// Destructor
        virtual ~CorrelatorSimulator();

        /// @brief Send the next correlator integration.
        ///
        /// @return true if there are more integrations in the dataset,
        ///         otherwise false. If false is returned, sendNext()
        ///         should not be called again.
        bool sendNext(void);

    private:

        // Baseline ID Map
        const BaselineMap itsBaselineMap;

        // Channel multiplication factor
        const unsigned int itsExpansionFactor;

        // Cursor (index) for the main table of the measurement set
        unsigned int itsCurrentRow;

        // Measurement set
        boost::scoped_ptr<casa::MeasurementSet> itsMS;

        // Port for output of metadata
        boost::scoped_ptr<askap::cp::VisPort> itsPort;
};

};
};
#endif
