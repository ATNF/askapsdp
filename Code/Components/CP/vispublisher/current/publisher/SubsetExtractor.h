/// @file SubsetExtractor.h
///
/// @copyright (c) 2014 CSIRO
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

#ifndef ASKAP_CP_ASKAP_SUBSETEXTRACTOR_H
#define ASKAP_CP_ASKAP_SUBSETEXTRACTOR_H

// ASKAPsoft includes
#include <vector>
#include <stdint.h>

// Local package includes
#include "publisher/SpdOutputMessage.h"
#include "publisher/InputMessage.h"

namespace askap {
namespace cp {
namespace vispublisher {

/// @brief Pure utility class that provides functionality to extract subsets of
/// the input message for sending on the ZeroMQ socket.
class SubsetExtractor {
    public:

        /// Extract a subset of the InputMessage.
        //
        /// @param[in] in   the input message from which the subset will be extracted.
        /// @param[in] beam the only extract data for this beam.
        /// @param[in] pol  the only extract data for this polarisation product.
        /// @return An SpdOutputMessage instance which corresponds to the data for a
        ///         specific beam and polarisation of the InputMessage.
        static SpdOutputMessage subset(const InputMessage& in, uint32_t beam,
                                    uint32_t pol);

    private:

        /// Creates filtered antenna index vectors.
        /// The InputMessgae will likely contain data for multiple beams in its
        /// antenna index vectors. This function builds ant1 and ant2 vectors for
        /// only the selected beam. For example given these input vectors in
        /// InputMessage:
        /// <CODE>
        /// beam = [0, 0, 0, 1, 1, 1]
        /// ant1 = [0, 0, 1, 0, 0, 1]
        /// ant2 = [0, 1, 1, 0, 1, 1]
        /// </CODE>
        /// the output should be if beam==1 is specified:
        /// <CODE>
        /// ant1out = [0, 0, 1]
        /// ant2out = [0, 1, 1]
        /// </CODE>
        ///
        /// @param[in] in   the input message to read the indexing vectors from. 
        /// @param[in] beam the beam number to extract vectors for.
        /// @param[out] ant1out the beam-n only subset of the antenna1 vector
        ///                     in the InputMessage.
        /// @param[out] ant2out the beam-n only subset of the antenna2 vector
        ///                     in the InputMessage.
        /// @return the size of the resulting ant1out and ant2out vectors (they are
        ///         both guaranteed to be of equal length.
        static uint32_t makeAntennaVectors(const InputMessage& in, uint32_t beam,
                                           std::vector<uint32_t>& ant1out,
                                           std::vector<uint32_t>& ant2out);

        /// Returns the element index of the first instance of "val" in the
        /// vector "v".
        static size_t indexOfFirst(const std::vector<uint32_t>& v, uint32_t val);

        /// For unit testing
        friend class SubsetExtractorTest;
};

}
}
}

#endif
