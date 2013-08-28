/// @file PreDifferResponse.cc
///
/// @copyright (c) 2009 CSIRO
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
#include <messages/PreDifferResponse.h>

// ASKAPsoft includes
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>

// Using
using namespace askap::cp;
using namespace askap::scimath;

PreDifferResponse::PreDifferResponse() : itsCount(0)
{
}

PreDifferResponse::~PreDifferResponse()
{
}

IMessage::MessageType PreDifferResponse::getMessageType(void) const
{
    return IMessage::PREDIFFER_RESPONSE;
}

/////////////////////////////////////////////////////////////////////
// Setters
/////////////////////////////////////////////////////////////////////
void PreDifferResponse::set_payloadType(PayloadType type)
{
    itsPayloadType = type;
}

void PreDifferResponse::set_count(int count)
{
    itsCount = count;
}

void PreDifferResponse::set_normalEquations(askap::scimath::INormalEquations::ShPtr ne)
{
    itsNe = ne;
}

/////////////////////////////////////////////////////////////////////
// Getters
/////////////////////////////////////////////////////////////////////
PreDifferResponse::PayloadType PreDifferResponse::get_payloadType(void) const
{
    return itsPayloadType;
}

int PreDifferResponse::get_count(void) const
{
    return itsCount;
}

askap::scimath::INormalEquations::ShPtr PreDifferResponse::get_normalEquations(void)
{
    return itsNe;
}

/////////////////////////////////////////////////////////////////////
// Serializers
/////////////////////////////////////////////////////////////////////
void PreDifferResponse::writeToBlob(LOFAR::BlobOStream& os) const
{
    os << static_cast<int>(itsPayloadType);
    if (itsPayloadType == RESULT) {
        os << itsCount;
        os << *itsNe;
    }
}

void PreDifferResponse::readFromBlob(LOFAR::BlobIStream& is)
{
    int payloadType;

    is >> payloadType;
    itsPayloadType = static_cast<PayloadType>(payloadType);

    if (itsPayloadType == RESULT) {
        itsNe = ImagingNormalEquations::ShPtr(new ImagingNormalEquations());
        is >> itsCount;
        is >> *itsNe;
    } else {
        itsCount = 0;
        itsNe.reset();
    }
}
