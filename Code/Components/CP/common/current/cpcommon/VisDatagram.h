/// @file VisDatagram.h
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

#ifndef ASKAP_CP_VISDATAGRAM_H
#define ASKAP_CP_VISDATAGRAM_H

#include <stdint.h>

namespace askap {
    namespace cp {

        /// @brief Encoding of a single precision complex floating point number
        /// for the correlator to central processor interface.
        struct FloatComplex
        {
            /// The real part of the complex number.
            float real;

            /// The imaginary part of the complex number.
            float imag;
        } __attribute__((__packed__));

        /// @brief Version number for the VisDatagram.
        static const uint32_t VISPAYLOAD_VERSION = 0x1;

        /// @brief Number of channels per slice in the VisDatagram. One
        /// VisDatagram will then contain data for N_CHANNELS_PER_SLICE channels.
        /// This is hardcoded to the standard ASKAP configuration so fixed size
        /// UDP datagrams can be used.
        static const uint32_t N_CHANNELS_PER_SLICE = 1026;

        /// @brief This structure specifies the UDP datagram which is sent from
        /// the correlator to the central processor. It contains all correlations
        /// for a single baseline, beam and coarse channel.
        struct VisDatagram
        {
            /// A version number for this structure. Also doubles as a magic
            /// number which can be used to verify if the datagram is of this
            /// type.
            uint32_t version;

            /// Slice number. Which slice of the channel space this packet
            /// relates to. For example, for a spectral window configuration of
            /// 16416 channels and N_CHANNELS_PER_SLICE of 1026 there will be
            /// a total of 16 slices.
            /// 
            /// This number is zero indexed, so the slices in the above example
            /// will be numbered 0 to 15.
            uint32_t slice;

            /// Timestamp - Binary Atomic Time (BAT). The number of microseconds
            /// since Modified Julian Day (MJD) = 0
            uint64_t timestamp;

            /// Baseline ID
            uint32_t baselineid;

            /// Beam ID
            uint32_t beamid;

            /// Visibilities
            FloatComplex vis[N_CHANNELS_PER_SLICE];
        } __attribute__((__packed__));

    };
};

#endif
