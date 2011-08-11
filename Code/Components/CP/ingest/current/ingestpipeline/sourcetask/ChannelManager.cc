/// @file ChannelManager.cc
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

// Include own header file first
#include "ChannelManager.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <map>
#include <iterator>
#include <limits>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "Common/ParameterSet.h"
#include "casa/Arrays/Vector.h"

ASKAP_LOGGER(logger, ".ChannelManager");

using namespace std;
using namespace askap;
using namespace askap::cp::ingest;

ChannelManager::ChannelManager(const LOFAR::ParameterSet& params)
{
    for (int i = 0; i < numeric_limits<int>::max(); ++i) {
        ostringstream ss;
        ss << "n_channels." << i;

        if (!params.isDefined(ss.str())) {
            break;
        }

        itsChannelMap[i] = params.getUint32(ss.str());
        ASKAPLOG_DEBUG_STR(logger, "Channel Mappings - Rank " << i
                               << " will handle " << itsChannelMap[i] << " channels");
    }
}

unsigned int ChannelManager::localNChannels(const int rank) const
{
    map<int, unsigned int>::const_iterator it = itsChannelMap.find(rank);

    if (it == itsChannelMap.end()) {
        ASKAPTHROW(AskapError, "No channel mapping for this rank");
    }

    return it->second;

}

casa::Vector<casa::Double> ChannelManager::localFrequencies(const int rank,
        const casa::Double startFreq,
        const casa::Double chanWidth) const
{
    casa::Vector<casa::Double> frequencies(localNChannels(rank));;

    // 1: Find the first frequency (freq of lowest channel) for this rank
    casa::Double firstFreq = startFreq;

    for (int i = 0; i < rank; ++i) {
        firstFreq += localNChannels(i) * chanWidth;
    }

    // 2: Populate the vector with the frequencies the process specified
    // by the "rank" parameter handles
    for (unsigned int i = 0; i < frequencies.size(); ++i) {
        frequencies(i) = firstFreq + (i * chanWidth);
    }

    return frequencies;
}
