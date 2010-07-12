/// @file SpectralLineWorkUnit.cc
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
#include <messages/SpectralLineWorkUnit.h>

// ASKAPsoft includes
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <casa/Arrays/Array.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>

// Using
using namespace askap::cp;

SpectralLineWorkUnit::SpectralLineWorkUnit() : itsChannelOffset(-1)
{
}

SpectralLineWorkUnit::~SpectralLineWorkUnit()
{
}

IMessage::MessageType SpectralLineWorkUnit::getMessageType(void) const
{
    return IMessage::SPECTRALLINE_WORKUNIT;
}

/////////////////////////////////////////////////////////////////////
// Setters
/////////////////////////////////////////////////////////////////////
void SpectralLineWorkUnit::set_payloadType(PayloadType type)
{
    itsPayloadType = type;
}

void SpectralLineWorkUnit::set_dataset(std::string dataset)
{
        itsDataset = dataset;
}

void SpectralLineWorkUnit::set_channelOffset(int offset)
{
    itsChannelOffset = offset;
}

/////////////////////////////////////////////////////////////////////
// Getters
/////////////////////////////////////////////////////////////////////
SpectralLineWorkUnit::PayloadType SpectralLineWorkUnit::get_payloadType(void) const
{
    return itsPayloadType;
}

std::string SpectralLineWorkUnit::get_dataset(void) const
{
        return itsDataset;
}

int SpectralLineWorkUnit::get_channelOffset(void) const
{
    return itsChannelOffset;
}

/////////////////////////////////////////////////////////////////////
// Serializers
/////////////////////////////////////////////////////////////////////
void SpectralLineWorkUnit::writeToBlob(LOFAR::BlobOStream& os) const
{
    os << static_cast<int>(itsPayloadType);
    os << itsDataset;
    os << itsChannelOffset;
}

void SpectralLineWorkUnit::readFromBlob(LOFAR::BlobIStream& is)
{
    int payloadType;

    is >> payloadType;
    is >> itsDataset;
    is >> itsChannelOffset;

    itsPayloadType = static_cast<PayloadType>(payloadType);
}
