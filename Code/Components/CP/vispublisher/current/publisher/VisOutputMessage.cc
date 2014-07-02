/// @file VisOutputMessage.cc
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
#include "publisher/VisOutputMessage.h"

// Include package level header file
#include "askap_vispublisher.h"

// System includes
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"

// Using
using namespace askap::cp::vispublisher;

VisOutputMessage::VisOutputMessage()
    : itsTimestamp(0), itsNBaselines(0), itsNBeams(0), itsNPols(0),
    itsChanBegin(0), itsChanEnd(0)
{
}

void VisOutputMessage::encode(zmq::message_t& msg) const
{
    // Preconditions
    ASKAPASSERT(itsAntenna1.size() == itsNBaselines);
    ASKAPASSERT(itsAntenna2.size() == itsNBaselines);
    ASKAPASSERT(itsAmplitudes.size() == itsNBaselines * itsNBeams * itsNPols);
    ASKAPASSERT(itsPhases.size() == itsNBaselines * itsNBeams * itsNPols);
    ASKAPASSERT(itsDelays.size() == itsNBaselines * itsNBeams * itsNPols);

    const size_t sz = sizeInBytes();
    msg.rebuild(sz);
    uint8_t* ptr = static_cast<uint8_t*>(msg.data());
    ptr = pushBack<uint64_t>(itsTimestamp, ptr);
    ptr = pushBack<uint32_t>(itsNBaselines, ptr);
    ptr = pushBack<uint32_t>(itsNBeams, ptr);
    ptr = pushBack<uint32_t>(itsNPols, ptr);
    ptr = pushBack<uint32_t>(itsChanBegin, ptr);
    ptr = pushBack<uint32_t>(itsChanEnd, ptr);
    ptr = pushBackVector<uint32_t>(itsAntenna1, ptr);
    ptr = pushBackVector<uint32_t>(itsAntenna2, ptr);
    ptr = pushBackVector<float>(itsAmplitudes, ptr);
    ptr = pushBackVector<float>(itsPhases, ptr);
    ptr = pushBackVector<float>(itsDelays, ptr);

    // Post-conditions
    ASKAPASSERT(ptr == static_cast<uint8_t*>(msg.data()) + sz);
}

size_t VisOutputMessage::sizeInBytes(void) const
{
    return sizeof (uint64_t)        // time
        + (5 * sizeof (uint32_t))   // nBaselines, nBeams, nPols, chanBegin, chanEnd
        + (itsAntenna1.size() * sizeof (uint32_t))
        + (itsAntenna2.size() * sizeof (uint32_t))
        + (itsAmplitudes.size() * sizeof (float))
        + (itsPhases.size() * sizeof (float))
        + (itsDelays.size() * sizeof (float));
}

template <typename T>
uint8_t* VisOutputMessage::pushBack(const T src, uint8_t* ptr)
{
    const size_t sz = sizeof (T);
    memcpy(ptr, &src, sz);
    return ptr + sz;
}

template <typename T>
uint8_t* VisOutputMessage::pushBackVector(const std::vector<T>& src, uint8_t* ptr)
{
    const size_t sz = src.size() * sizeof (T);
    memcpy(ptr, &src[0], sz);
    return ptr + sz;
}
