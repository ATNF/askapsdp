/// @file VisPayload.h
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

#ifndef ASKAP_CP_VISPAYLOAD_H
#define ASKAP_CP_VISPAYLOAD_H

namespace askap {
    namespace cp {

        /// @brief Encoding of a single precision complex floating point number
        /// for the correlator to central processor interface.
        struct FloatComplex
        {
            float real;
            float imag;
        };

        /// @brief Version number for the VisPayload.
        static const unsigned int VISPAYLOAD_VERSION = 0x1;

        /// @brief Number of fine channels per coarse channel in the VisPayload.
        /// This is hardcoded to the standard ASKAP configuration so fixed size
        /// UDP datagrams can be used.
        static const unsigned int N_FINE_PER_COARSE = 54;

        /// @brief Number of polarisations present in the VisPayload. This is
        /// hardcoded to the standard ASKAP configuration so fixed size UDP
        /// datagrams can be used.
        static const unsigned int N_POL = 4;

        /// @brief This structure specifies the UDP datagram which is sent from
        /// the correlator to the central processor. It contains all correlations
        /// for a single baseline, beam and coarse channel.
        struct VisPayload
        {
            /// A version number for this structure. Also doubles as a magic
            /// number which can be used to verify if the datagram is of this
            /// type.
            unsigned int version;

            /// Timestamp - Binary Atomic Time (BAT). The number of microseconds
            /// since Modified Julian Day (MJD) = 0
            unsigned long timestamp;

            /// Coarse Channel. Which coarse channel this block of data relates
            /// to. This is a one based number and should be in the range of
            /// 1 to 304 for ASKAP.
            unsigned int coarseChannel;

            /// First antenna
            unsigned int antenna1;

            /// Second antenna
            unsigned int antenna2;

            /// First beam
            unsigned int beam1;

            /// Second beam
            unsigned int beam2;

            /// Visibilities
            FloatComplex vis[N_FINE_PER_COARSE * N_POL];

            /// The number of voltage samples that made up the visibility for this
            /// integration. This has the same dimension as "vis. i.e. one nSamples
            /// value per visibility in the "vis" array. An nSamples value of zero for
            /// any channel/polarization indicates that visibility has been flagged by
            /// the correlator as bad.
            unsigned char nSamples[N_FINE_PER_COARSE * N_POL];
        };

    };
};

#endif
