/// @file PreDifferRequest.cc
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
#include <messages/PreDifferRequest.h>

// ASKAPsoft includes
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <casa/Arrays/Array.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>

// Using
using namespace askap::cp;

PreDifferRequest::PreDifferRequest()
{
}

PreDifferRequest::~PreDifferRequest()
{
}

IMessage::MessageType PreDifferRequest::getMessageType(void) const
{
    return IMessage::PREDIFFER_REQUEST;
}

/////////////////////////////////////////////////////////////////////
// Setters
/////////////////////////////////////////////////////////////////////
void PreDifferRequest::set_payloadType(PayloadType type)
{
    itsPayloadType = type;
}

void PreDifferRequest::set_dataset(std::string dataset)
{
    itsDataset = dataset;
}

/////////////////////////////////////////////////////////////////////
// Getters
/////////////////////////////////////////////////////////////////////
PreDifferRequest::PayloadType PreDifferRequest::get_payloadType(void) const
{
    return itsPayloadType;
}

std::string PreDifferRequest::get_dataset(void) const
{
    return itsDataset;
}

/////////////////////////////////////////////////////////////////////
// Serializers
/////////////////////////////////////////////////////////////////////
void PreDifferRequest::writeToBlob(LOFAR::BlobOStream& os) const
{
    os << static_cast<int>(itsPayloadType);
    os << itsDataset;
}

void PreDifferRequest::readFromBlob(LOFAR::BlobIStream& is)
{
    int payloadType;

    is >> payloadType;
    is >> itsDataset;

    itsPayloadType = static_cast<PayloadType>(payloadType);
}
