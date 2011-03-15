/// @file UVChannelDataIterator.cc
/// @brief
///
/// @copyright (c) 2011 CSIRO
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
#include "UVChannelDataIterator.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "dataaccess/DataAccessError.h"
#include "dataaccess/DirectionConverter.h"
#include "cpcommon/VisChunk.h"
#include "dataaccess/MemBufferDataAccessor.h"

// Local includes
#include "uvchannel/UVChannelConfig.h"
#include "uvchannel/uvdataaccess/UVChannelDataSource.h"
#include "uvchannel/uvdataaccess/UVChannelDataSelector.h"
#include "uvchannel/uvdataaccess/UVChannelDataConverter.h"
#include "uvchannel/uvdataaccess/UVChannelReceiver.h"

// Using
using namespace casa;
using namespace askap;
using namespace askap::accessors;
using namespace askap::cp::channels;

UVChannelDataIterator::UVChannelDataIterator(const UVChannelConfig& channelConfig,
        const std::string& channelName,
        const boost::shared_ptr<const UVChannelDataSelector> &sel,
        const boost::shared_ptr<const UVChannelDataConverter> &conv)
        : UVChannelConstDataIterator(channelConfig, channelName, sel, conv)
{
}

casa::Bool UVChannelDataIterator::next()
{
    casa::Bool val = UVChannelConstDataIterator::next();
    itsAccessor.reset(new MemBufferDataAccessor(*itsAccessor));
    return val;
}

IDataAccessor& UVChannelDataIterator::operator*() const
{
    return *itsAccessor;
}

IDataAccessor* UVChannelDataIterator::operator->() const
{
    return itsAccessor.get();
}

void UVChannelDataIterator::chooseBuffer(const std::string &bufferID)
{
    ASKAPTHROW(AskapError, "UVChannelDataIterator::chooseBuffer() not supported");
}

void UVChannelDataIterator::chooseOriginal()
{
    ASKAPTHROW(AskapError, "UVChannelDataIterator::chooseOriginal() not supported");
}

IDataAccessor& UVChannelDataIterator::buffer(const std::string &bufferID) const
{
    ASKAPTHROW(AskapError, "UVChannelDataIterator::buffer() not supported");
}
