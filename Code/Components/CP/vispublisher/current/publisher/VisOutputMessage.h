/// @file VisOutputMessage.h
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

#ifndef ASKAP_CP_VISPUBLISHER_VISOUTPUTMESSAGE_T
#define ASKAP_CP_VISPUBLISHER_VISOUTPUTMESSAGE_T

// System includes
#include <vector>
#include <stdint.h>

// ASKAPsoft includes
#include <zmq.hpp>

// Local package includes
#include "VisElement.h"

namespace askap {
namespace cp {
namespace vispublisher {

/// @brief Encapsulates the message published by the vispublisher.
/// The accessor methods return references to the member variables
/// for reasons of performance. This allows the message to be populated
/// without any copying.
class VisOutputMessage {
    public:
        /// @brief Constructor.
        VisOutputMessage();

        /// Encodes this instance of VisOutputMessage to the zmq::message passed.
        ///
        /// @param[out] msg     the message object to populate. This message
        ///                     will be rebuilt (resized) and then its contents
        ///                     will be populated with a serialised instance of
        ///                     this class.
        void encode(zmq::message_t& msg) const;

        /// Binary Atomic Time (BAT) of the correlator integration midpoint.
        /// The number of microseconds since Modified Julian Day (MJD) = 0
        uint64_t& timestamp(void) { return itsTimestamp; };

        /// The first (inclusive) channel number (one based) of the range of
        /// channels used to form the products (i.e. tvchan)
        uint32_t& chanBegin(void) { return itsChanBegin; };

        /// The last (inclusive) channel number (one based) of the range of
        /// channels used to form the products (i.e. tvchan)
        uint32_t& chanEnd(void) { return itsChanEnd; };

        std::vector<VisElement>& data() { return itsData; };

    private:

        /// Returns the number of bytes required to encode this message.
        /// This is used by encode() to build a message object.
        size_t sizeInBytes(void) const;

        template <typename T>
        static uint8_t* pushBack(const T src, uint8_t* ptr);

        static uint8_t* pushBackVisElements(const std::vector<VisElement>& src, uint8_t* ptr);

        /// Binary Atomic Time (BAT) of the correlator integration midpoint.
        /// The number of microseconds since Modified Julian Day (MJD) = 0
        uint64_t itsTimestamp;

        /// Chan Begin
        uint32_t itsChanBegin;

        /// Chan End
        uint32_t itsChanEnd;

        /// VisElement vector
        std::vector<VisElement> itsData;
};

}
}
}

#endif
