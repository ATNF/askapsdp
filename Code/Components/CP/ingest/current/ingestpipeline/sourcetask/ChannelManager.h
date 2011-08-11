/// @file ChannelManager.h
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

#ifndef ASKAP_CP_INGEST_CHANNELMANAGER_H
#define ASKAP_CP_INGEST_CHANNELMANAGER_H

// System includes
#include <map>

// ASKAPsoft includes
#include "casa/aips.h"
#include "Common/ParameterSet.h"
#include "casa/Arrays/Vector.h"

namespace askap {
namespace cp {
namespace ingest {

/// Encapsulates management of spectral channels.
class ChannelManager {
    public:
        /// @brief Constructor
        /// The input parameter set will describe the number of channels
        /// handled by each node. For example to describe two nodes, each
        /// handling 1024 spectral channels the following parameters would
        /// be used:
        /// @verbatim
        /// n_channels.0 = 1024
        /// n_channels.1 = 1024
        /// @endverbatim
        ///
        /// @param[in] params the input parameter set.
        ChannelManager(const LOFAR::ParameterSet& params);

        /// Returns the number of spectral channels the process
        /// specified by the "rank" parameter handles.
        ///
        /// @param[in] rank the MPI rank of the process for which
        ///                 information is desired.
        /// @return the number of spectral channels the process
        ///     specified by the "rank" parameter handles.
        unsigned int localNChannels(const int rank) const;

        /// Returns a vector containing the frequencies of the spectral
        /// channels handled by the process specified by the "rank"
        /// parameter.
        /// @note The unit for startFreq and chanWidth should be the same
        /// and the unit for the return value will also be the same as
        /// for those parameters.
        ///
        /// @param[in] rank the MPI rank of the process for which
        ///                 information is desired.
        /// @param[in] startFreq    the frequency of the lowest numbered
        ///                         channel for the whole system.
        /// @param[in] chanWidth    the width of the spectral channels. All
        ///                         channels thus have the same width given
        ///                         this is a scalar parameter.
        casa::Vector<casa::Double> localFrequencies(const int rank,
                const casa::Double startFreq,
                const casa::Double chanWidth) const;

    private:

        // Tracks the number of channels each process handles
        // first: is the process rank, second: is the number
        // of spectral channels it handles
        std::map<int, unsigned int> itsChannelMap;
};

}
}
}

#endif
