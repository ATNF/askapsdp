/// @file SpdOutputMessage.h
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

#ifndef ASKAP_CP_VISPUBLISHER_SPDOUTPUTMESSAGE_T
#define ASKAP_CP_VISPUBLISHER_SPDOUTPUTMESSAGE_T

// System includes
#include <complex>
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
class SpdOutputMessage {
    public:
        /// @brief Constructor.
        SpdOutputMessage();

        /// Encodes this instance of SpdOutputMessage to the zmq::message passed.
        ///
        /// @param[out] msg     the message object to populate. This message
        ///                     will be rebuilt (resized) and then its contents
        ///                     will be populated with a serialised instance of
        ///                     this class.
        void encode(zmq::message_t& msg) const;

        /// Binary Atomic Time (BAT) of the correlator integration midpoint.
        /// The number of microseconds since Modified Julian Day (MJD) = 0
        uint64_t& timestamp(void) { return itsTimestamp; };

        /// Beam ID (zero based)
        uint32_t& beamId(void) { return itsBeamId; };

        /// Polarisation - 0=XX, 1=XY, 2=YX, 3=YY
        uint32_t& polId(void) { return itsPolarisationId; };

        /// Number of spectral channels
        uint32_t& nChannels(void) { return itsNChannels; };

        /// Channel width (in Hz)
        double& chanWidth(void) { return itsChanWidth; };

        /// Freqency (in Hz) for each of the nChannels
        std::vector<double>& frequency(void) { return itsFrequency; };

        /// Number of baselines
        uint32_t& nBaselines(void) { return itsNBaselines; };

        /// Antenna 1 - Maps for baseline index to antenna index
        std::vector<uint32_t>& antenna1(void) { return itsAntenna1; };

        /// Antenna 2 - Maps for baseline index to antenna index
        std::vector<uint32_t>& antenna2(void) { return itsAntenna2; };

        /// Visibilities (nChannels * nBaselines)
        std::vector< std::complex<float> >& visibilities(void) { return itsVisibilities; };

        /// Flag (nChannels * nBaselines)
        /// 0=Visibility not flagged, 1=Visibility flagged
        std::vector< uint8_t >& flag(void) { return itsFlag; };

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

        /// Beam ID (zero based)
        uint32_t itsBeamId;

        /// Polarisation - 0=XX, 1=XY, 2=YX, 3=YY
        uint32_t itsPolarisationId; 

        /// Number of spectral channels
        uint32_t itsNChannels;

        /// Channel width (in Hz)
        double itsChanWidth;

        /// Freqency (in Hz) for each of the nChannels
        std::vector<double> itsFrequency;

        /// Number of baselines
        uint32_t itsNBaselines;

        /// Antenna 1 - Maps for baseline index to antenna index
        std::vector<uint32_t> itsAntenna1;

        /// Antenna 2 - Maps for baseline index to antenna index
        std::vector<uint32_t> itsAntenna2;

        /// Visibilities (nChannels * nBaselines)
        std::vector< std::complex<float> > itsVisibilities;

        /// Flag (nChannels * nBaselines)
        /// 0=Visibility not flagged, 1=Visibility flagged
        std::vector<uint8_t> itsFlag;
};

}
}
}

#endif
