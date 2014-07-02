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

/// @brief Pure utility class that provides functionality to extract subsets of
/// the input message for sending on the ZeroMQ socket.
class VisMessageBuilder {
    public:

        static VisOutputMessage build(const InputMessage& in,
                                      uint32_t tvChanBegin,
                                      uint32_t tvChanEnd);

    private:

        static std::pair<float, float> ampAndPhase(const std::vector< std::complex<float> >& vis,
                                                   const std::vector<bool>& flag);

        static float calcDelay(const std::vector< std::complex<float> >& vis,
                               double chanWidth);
};

}
}
}

#endif
