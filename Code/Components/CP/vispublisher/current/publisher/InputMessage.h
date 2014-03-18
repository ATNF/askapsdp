/// @file InputMessage.h
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

#ifndef ASKAP_CP_VISPUBLISHER_INPUTMESSAGE_T
#define ASKAP_CP_VISPUBLISHER_INPUTMESSAGE_T

// System includes
#include <complex>
#include <vector>
#include <stdint.h>

// ASKAPsoft includes
#include <boost/asio.hpp>

// Local package includes

namespace askap {
namespace cp {
namespace vispublisher {

/// @brief
class InputMessage {
    public:
        InputMessage();

        static InputMessage build(boost::asio::ip::tcp::socket& socket);

        uint64_t& timestamp(void);
        const uint64_t& timestamp(void) const;

        uint32_t& nRow(void);
        const uint32_t& nRow(void) const;

        uint32_t& nPol(void);
        const uint32_t& nPol(void) const;

        uint32_t& nChannels(void);
        const uint32_t& nChannels(void) const;

        double& chanWidth(void);
        const double& chanWidth(void) const;

        std::vector<double>& frequency(void);
        const std::vector<double>& frequency(void) const;

        std::vector<uint32_t>& antenna1(void);
        const std::vector<uint32_t>& antenna1(void) const;

        std::vector<uint32_t>& antenna2(void);
        const std::vector<uint32_t>& antenna2(void) const;

        std::vector<uint32_t>& beam(void);
        const std::vector<uint32_t>& beam(void) const;

        std::vector<uint32_t>& stokes(void);
        const std::vector<uint32_t>& stokes(void) const;

        std::vector< std::complex<float> >& visibilities(void);
        const std::vector< std::complex<float> >& visibilities(void) const;

        std::vector<uint8_t>& flag(void);
        const std::vector<uint8_t>& flag(void) const;

    private:

        template <typename T>
        static T read(boost::asio::ip::tcp::socket& socket);

        template <typename T>
        static std::vector<T> readVector(boost::asio::ip::tcp::socket& socket, size_t n);

        uint32_t itsNRow;

        // Number of spectral channels
        uint32_t itsNChannel;

        uint32_t itsNPol;

        // Binary Atomic Time (BAT) of the correlator integration midpoint.
        // The number of microseconds since Modified Julian Day (MJD) = 0
        uint64_t itsTimestamp;

        // Channel width (in Hz)
        double itsChanWidth;

        // Freqency (in Hz) for each of the nChannels
        std::vector<double> itsFrequency;

        // Antenna 1
        std::vector<uint32_t> itsAntenna1;

        // Antenna 2
        std::vector<uint32_t> itsAntenna2;

        // Beam
        std::vector<uint32_t> itsBeam;

        // Stokes
        std::vector<uint32_t> itsStokes;

        // Visibilities (nChannels * nPols * nRows)
        std::vector< std::complex<float> > itsVisibilities;

        // Flag (nChannels * nPols * nRows)
        // 0=Visibility not flagged, 1=Visibility flagged
        std::vector<uint8_t> itsFlag;
};

}
}
}

#endif
