/// @file VisMessageBuilder.h
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

#ifndef ASKAP_CP_ASKAP_VISMESSAGEBUILDER_H
#define ASKAP_CP_ASKAP_VISMESSAGEBUILDER_H

// ASKAPsoft includes
#include <vector>
#include <utility>
#include <complex>
#include <stdint.h>

// Local package includes
#include "publisher/VisOutputMessage.h"
#include "publisher/InputMessage.h"

namespace askap {
namespace cp {
namespace vispublisher {

/// @brief Pure utility class used for transforming input visibilities into
/// Vis summary data (amplitude, phase, delay).
class VisMessageBuilder {
    public:

        /// Build a vis output message from a given input message.
        ///
        /// @param[in] in   the input message.
        /// @param[in] tvChanBegin  the first channel of the channel range to
        ///                         be used to calculate statistics. The range
        ///                         is inclusive of this channel.
        /// @param[in] tvChanEnd    the last channel of the channel range to
        ///                         be used to calculate statistics. The range
        ///                         is inclusive of this channel.
        static VisOutputMessage build(const InputMessage& in,
                                      uint32_t tvChanBegin,
                                      uint32_t tvChanEnd);

    private:

        /// Calculate amplitude and phase
        static std::pair<double, double> ampAndPhase(
                const std::vector< std::complex<float> >& vis,
                const std::vector<bool>& flag);

        /// Estimate delays
        static double calcDelay(const std::vector< std::complex<float> >& vis,
                                const std::vector<bool>& flag,
                                double chanWidth);

        /// Average channels in vis
        /// @param[in] vis              input visibilities
        /// @param[in] numberToAverage  number of visibilities to average
        ///                             together to form one.
        /// @return a vector of length (vis.size() / numberToAverage)
        static std::vector< std::complex<float> > averageChannels(
            const std::vector< std::complex<float> >& vis,
            const std::vector<bool>& flag,
            uint32_t numberToAverage);
};

}
}
}

#endif
