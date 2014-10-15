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

namespace askap {
namespace cp {
namespace vispublisher {

/// @brief Encapsulates the message send from the ingest pipeline.
/// The accessor methods return references to the member variables
/// for reasons of performance.
class InputMessage {
    public:
        /// @brief Constructor
        InputMessage();

        /// @brief Builds an instances of InputMessage by deserialising input
        /// received on the supplied socket object.
        /// @param[in] socket   the socket to read a stream of bytes needed to
        ///                     build the InputMessage.
        /// @return a newly constructed InputMessage
        /// @throws AskapError  if an error occurs while reading from the
        ///                     network socket.
        static InputMessage build(boost::asio::ip::tcp::socket& socket);

        uint64_t& timestamp(void);
        const uint64_t& timestamp(void) const;

        uint32_t& scan(void);
        const uint32_t& scan(void) const;

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

        /// Index into the InputMessage visibilities or flag vectors,
        /// converting a 3D index into a 1D index.
        ///
        /// @param[in] row
        /// @param[in] chan
        /// @param[in] pol
        /// @return array index valid for either the visibilities or
        ///         flag vectors
        size_t index(size_t row, size_t chan, size_t pol) const;

    private:

        /// Constructs an object of type T from bytes read from the network socket.
        ///
        /// This class will read sizeof (T) bytes, cast it to a type T and return it.
        ///
        /// @param[in] socket   the socket to read a stream of bytes needed to
        ///                     build the returned instance.
        /// @return a newly constructed instance of type T
        /// @throws AskapError  if an error occurs while reading from the
        ///                     network socket.
        template <typename T>
        static T read(boost::asio::ip::tcp::socket& socket);

        /// Constructs a vector of elements of type T from bytes read from the
        /// network socket.
        ///
        /// This class will read n * sizeof (T) bytes into a vector of type T
        /// and length n.
        ///
        /// @param[in] socket   the socket to read a stream of bytes needed to
        ///                     build the returned instance.
        /// @return a newly constructed instance of type T
        /// @throws AskapError  if an error occurs while reading from the
        ///                     network socket.
        template <typename T>
        static std::vector<T> readVector(boost::asio::ip::tcp::socket& socket, size_t n);

        /// Number of rows in the datsset
        uint32_t itsNRow;

        /// Number of spectral channels
        uint32_t itsNChannel;

        /// Number of polarisation products
        uint32_t itsNPol;

        /// Binary Atomic Time (BAT) of the correlator integration midpoint.
        /// The number of microseconds since Modified Julian Day (MJD) = 0
        uint64_t itsTimestamp;

        /// Scan ID
        uint32_t itsScan;

        /// Channel width (in Hz)
        double itsChanWidth;

        /// Frequency (in Hz) for each of the nChannels. The length of this vector
        /// will be equal to itsNChannel
        std::vector<double> itsFrequency;

        /// Maps from row number (element index) to antenna index number (element
        /// value) for antenna 1.
        std::vector<uint32_t> itsAntenna1;

        /// Maps from row number (element index) to antenna index number (element
        /// value) for antenna 2.
        std::vector<uint32_t> itsAntenna2;

        /// Maps from row number (element index) to beam index number (element
        /// value).
        std::vector<uint32_t> itsBeam;

        /// Maps from index number to stokes type. The element values map:
        /// 0=XX, 1=XY, 2=YX, 3=YY
        std::vector<uint32_t> itsStokes;

        /// Visibilities (nChannels * nPols * nRows)
        std::vector< std::complex<float> > itsVisibilities;

        /// Flag (nChannels * nPols * nRows)
        /// 0=Visibility not flagged, 1=Visibility flagged
        std::vector<uint8_t> itsFlag;
};

}
}
}

#endif
