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
#include "publisher/OutputMessage.h"
#include "publisher/InputMessage.h"

namespace askap {
namespace cp {
namespace vispublisher {

/// @brief TODO: Write documentation...
class SubsetExtractor {
    public:

        static OutputMessage subset(const InputMessage& in, uint32_t beam, uint32_t pol);

    private:

        static uint32_t makeAntennaVectors(const InputMessage& in, uint32_t beam,
                                           std::vector<uint32_t>& ant1out,
                                           std::vector<uint32_t>& ant2out);

        static size_t inIndex(size_t row, size_t chan, size_t pol, size_t nChannels, size_t nPol);

        static size_t indexOfFirst(const std::vector<uint32_t>& v, uint32_t val);

        friend class SubsetExtractorTest;
};

}
}
}

#endif
