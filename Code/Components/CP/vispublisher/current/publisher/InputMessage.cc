/// @file InputMessage.cc
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

// Include own header file first
#include "publisher/InputMessage.h"

// Include package level header file
#include "askap_vispublisher.h"

// System includes
#include <complex>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"

// Using
using namespace askap::cp::vispublisher;

ASKAP_LOGGER(logger, ".InputMessage");

InputMessage::InputMessage()
{
}

InputMessage InputMessage::build(boost::asio::ip::tcp::socket& socket)
{
    InputMessage msg;
    msg.itsNRow = read<uint32_t>(socket);
    msg.itsNChannel = read<uint32_t>(socket);
    msg.itsNPol = read<uint32_t>(socket);
    msg.itsTimestamp = read<uint64_t>(socket);
    msg.itsChanWidth = read<double>(socket);
    msg.itsFrequency = readVector<double>(socket, msg.itsNChannel);
    msg.itsAntenna1 = readVector<uint32_t>(socket, msg.itsNRow);
    msg.itsAntenna2 = readVector<uint32_t>(socket, msg.itsNRow);
    msg.itsBeam = readVector<uint32_t>(socket, msg.itsNRow);
    msg.itsStokes = readVector<uint32_t>(socket, msg.itsNPol);
    const size_t cubeSize = msg.itsNRow * msg.itsNChannel * msg.itsNPol;
    msg.itsVisibilities = readVector< std::complex<float> >(socket, cubeSize);
    msg.itsFlag = readVector<uint8_t>(socket, cubeSize);
    return msg;
}

uint32_t& InputMessage::nRow(void)
{
    return itsNRow;
}

const uint32_t& InputMessage::nRow(void) const
{
    return itsNRow;
}

uint32_t& InputMessage::nPol(void)
{
    return itsNPol;
}

const uint32_t& InputMessage::nPol(void) const
{
    return itsNPol;
}

uint32_t& InputMessage::nChannels(void)
{
    return itsNChannel;
}

const uint32_t& InputMessage::nChannels(void) const
{
    return itsNChannel;
}

uint64_t& InputMessage::timestamp(void)
{
    return itsTimestamp;
}

const uint64_t& InputMessage::timestamp(void) const
{
    return itsTimestamp;
}

double& InputMessage::chanWidth(void)
{
    return itsChanWidth;
}

const double& InputMessage::chanWidth(void) const
{
    return itsChanWidth;
}

std::vector<double>& InputMessage::frequency(void)
{
    return itsFrequency;
}

const std::vector<double>& InputMessage::frequency(void) const
{
    return itsFrequency;
}

std::vector<uint32_t>& InputMessage::antenna1(void)
{
    return itsAntenna1;
}

const std::vector<uint32_t>& InputMessage::antenna1(void) const
{
    return itsAntenna1;
}

std::vector<uint32_t>& InputMessage::antenna2(void)
{
    return itsAntenna2;
}

const std::vector<uint32_t>& InputMessage::antenna2(void) const
{
    return itsAntenna2;
}

std::vector<uint32_t>& InputMessage::beam(void)
{
    return itsBeam;
}

const std::vector<uint32_t>& InputMessage::beam(void) const
{
    return itsBeam;
}

std::vector<uint32_t>& InputMessage::stokes(void)
{
    return itsStokes;
}

const std::vector<uint32_t>& InputMessage::stokes(void) const
{
    return itsStokes;
}

std::vector< std::complex<float> >& InputMessage::visibilities(void)
{
    return itsVisibilities;
}

const std::vector< std::complex<float> >& InputMessage::visibilities(void) const
{
    return itsVisibilities;
}

std::vector<uint8_t>& InputMessage::flag(void)
{
    return itsFlag;
}

const std::vector<uint8_t>& InputMessage::flag(void) const
{
    return itsFlag;
}

template <typename T>
T InputMessage::read(boost::asio::ip::tcp::socket& socket)
{
    T val;
    size_t sz = sizeof (T);
    boost::system::error_code error;
    size_t len = boost::asio::read(socket, boost::asio::buffer(&val, sz), error);
    if (error) {
        ASKAPTHROW(AskapError, error.message());
    }
    ASKAPCHECK(len == sz, "Short read - Expected: " << sz << " Actual: " << len);
    return val;
}

template <typename T>
std::vector<T> InputMessage::readVector(boost::asio::ip::tcp::socket& socket, size_t n)
{
    std::vector<T> vec(n);
    boost::system::error_code error;
    size_t len = boost::asio::read(socket, boost::asio::buffer(boost::asio::buffer(vec)), error);
    if (error) {
        ASKAPTHROW(AskapError, error.message());
    }
    ASKAPCHECK(len == (n * sizeof (T)), "Short read - Expected: " << n << " x "
            << sizeof (T) << " Actual: " << len);
    return vec;
}
