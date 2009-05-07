/// @file CleanResponse.cc
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
#include <messages/CleanResponse.h>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>
#include <casa/Arrays/Array.h>

// Using
using namespace askap;
using namespace askap::cp;

ASKAP_LOGGER(logger, ".CleanResponse");

CleanResponse::CleanResponse() : itsPayloadType(READY),
    itsPatchId(-1), itsStrengthOptimum(-1)
{
}

CleanResponse::~CleanResponse()
{
}

IMessage::MessageType CleanResponse::getMessageType(void) const
{
    return IMessage::CLEAN_RESPONSE;
}

void CleanResponse::set_patchId(int patchId)
{
    itsPatchId = patchId;
}

void CleanResponse::set_patch(const casa::Array<float>& patch)
{
    itsPatch = patch;
}

void CleanResponse::set_strengthOptimum(double strengthOptimum)
{
    itsStrengthOptimum = strengthOptimum;
}

void CleanResponse::set_payloadType(PayloadType type)
{
    itsPayloadType = type;
}

int CleanResponse::get_patchId(void) const
{
    return itsPatchId;
}

const casa::Array<float>& CleanResponse::get_patch(void) const
{
    return itsPatch;
}

casa::Array<float>& CleanResponse::get_patch(void)
{
    return itsPatch;
}

double CleanResponse::get_strengthOptimum(void) const
{
    return itsStrengthOptimum;
}

CleanResponse::PayloadType CleanResponse::get_payloadType(void) const
{
    return itsPayloadType;
}

void CleanResponse::writeToBlob(LOFAR::BlobOStream& os) const
{
    ASKAPLOG_INFO_STR(logger, "CleanResponse::writeToBlob");
    os << static_cast<int>(itsPayloadType);

    if (itsPayloadType == RESULT) {
        os << itsPatchId;
        os << itsPatch;
        os << itsStrengthOptimum;
    }
}

void CleanResponse::readFromBlob(LOFAR::BlobIStream& is)
{
    ASKAPLOG_INFO_STR(logger, "CleanResponse::readFromBlob");
    int payloadType;
    is >> payloadType;
    itsPayloadType = static_cast<PayloadType>(payloadType);

    if (itsPayloadType == RESULT) {
        is >> itsPatchId;
        is >> itsPatch;
        is >> itsStrengthOptimum;
    }
}
