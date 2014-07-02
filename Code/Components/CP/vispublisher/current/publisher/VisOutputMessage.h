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

        /// Number of baselines
        uint32_t& nBaselines(void) { return itsNBaselines; };
        
        /// Number of beams
        uint32_t& nBeams(void) { return itsNBeams; };

        /// Number of polarisations
        uint32_t& nPols(void) { return itsNPols; };

        /// The first (inclusive) channel number (one based) of the range of
        /// channels used to form the products (i.e. tvchan)
        uint32_t& chanBegin(void) { return itsChanBegin; };

        /// The last (inclusive) channel number (one based) of the range of
        /// channels used to form the products (i.e. tvchan)
        uint32_t& chanEnd(void) { return itsChanEnd; };

        /// Antenna 1 - Maps for baseline index to antenna index
        std::vector<uint32_t>& antenna1(void) { return itsAntenna1; };

        /// Antenna 2 - Maps for baseline index to antenna index
        std::vector<uint32_t>& antenna2(void) { return itsAntenna2; };

        /// Average amplitudes
        /// Units: Pseudo Jy
        std::vector<float>& amplitudes(void) { return itsAmplitudes; };

        /// Average phase angle
        /// Units: Degrees
        std::vector<float>& phases(void) { return itsPhases; };

        /// Delays
        /// Units: Seconds
        std::vector<float>& delays(void) { return itsDelays; };

    private:

        /// Returns the number of bytes required to encode this message.
        /// This is used by encode() to build a message object.
        size_t sizeInBytes(void) const;

        template <typename T>
        static uint8_t* pushBack(const T src, uint8_t* ptr);

        template <typename T>
        static uint8_t* pushBackVector(const std::vector<T>& src, uint8_t* ptr);

        /// Binary Atomic Time (BAT) of the correlator integration midpoint.
        /// The number of microseconds since Modified Julian Day (MJD) = 0
        uint64_t itsTimestamp;

        /// Number of baselines
        uint32_t itsNBaselines;
        
        /// Number of beams
        uint32_t itsNBeams;

        /// Number of polarisations
        uint32_t itsNPols;

        /// Chan Begin
        uint32_t itsChanBegin;

        /// Chan End
        uint32_t itsChanEnd;

        /// Antenna 1 - Maps for baseline index to antenna index
        std::vector<uint32_t> itsAntenna1;

        /// Antenna 2 - Maps for baseline index to antenna index
        std::vector<uint32_t> itsAntenna2;

        /// Amplitudes
        std::vector<float> itsAmplitudes;

        /// Phases
        std::vector<float> itsPhases;

        /// Delays
        std::vector<float> itsDelays;
};

}
}
}

#endif
