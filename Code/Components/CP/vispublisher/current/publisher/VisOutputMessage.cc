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
    : itsTimestamp(0), itsChanBegin(0), itsChanEnd(0)
{
}

void VisOutputMessage::encode(zmq::message_t& msg) const
{
    const size_t sz = sizeInBytes();
    msg.rebuild(sz);
    uint8_t* ptr = static_cast<uint8_t*>(msg.data());
    ptr = pushBack<uint64_t>(itsTimestamp, ptr);
    ptr = pushBack<uint32_t>(itsChanBegin, ptr);
    ptr = pushBack<uint32_t>(itsChanEnd, ptr);
    ptr = pushBack<uint32_t>(itsData.size(), ptr);
    ptr = pushBackVisElements(itsData, ptr);

    // Post-conditions
    ASKAPASSERT(ptr == static_cast<uint8_t*>(msg.data()) + sz);
}

size_t VisOutputMessage::sizeInBytes(void) const
{
    const size_t dataSize = itsData.size() * ((4 * sizeof(uint32_t))
            + (3 * sizeof(double)));

    return sizeof (uint64_t)        // time
        + (3 * sizeof (uint32_t))   // chanBegin, chanEnd, nElements
        + dataSize;
}

template <typename T>
uint8_t* VisOutputMessage::pushBack(const T src, uint8_t* ptr)
{
    const size_t sz = sizeof (T);
    memcpy(ptr, &src, sz);
    return ptr + sz;
}

uint8_t* VisOutputMessage::pushBackVisElements(const std::vector<VisElement>& src, uint8_t* ptr)
{
    uint8_t* p = ptr;
    for (size_t i = 0; i < src.size(); ++i) {
        p = pushBack<uint32_t>(src[i].beam, p);
        p = pushBack<uint32_t>(src[i].antenna1, p);
        p = pushBack<uint32_t>(src[i].antenna2, p);
        p = pushBack<uint32_t>(src[i].pol, p);
        p = pushBack<double>(src[i].amplitude, p);
        p = pushBack<double>(src[i].phase, p);
        p = pushBack<double>(src[i].delay, p);
    }
    return p;
}
