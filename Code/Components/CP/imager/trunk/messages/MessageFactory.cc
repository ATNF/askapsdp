/// @file MessageFactory.cc
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
#include <messages/MessageFactory.h>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <messages/IMessage.h>

// Message header files
#include <messages/CleanRequest.h>
#include <messages/CleanResponse.h>

// Using
using namespace askap;
using namespace askap::cp;

ASKAP_LOGGER(logger, ".MessageFactory");

MessageFactory::MessageFactory()
{
}

MessageFactory::~MessageFactory()
{
}

IMessageSharedPtr MessageFactory::create(const IMessage::MessageType& type)
{
    IMessageSharedPtr msg;

    switch (type) {
        case IMessage::CLEAN_REQUEST:
            msg.reset(new CleanRequest);
            break;

        case IMessage::CLEAN_RESPONSE:
            msg.reset(new CleanResponse);
            break;

        default:
            ASKAPTHROW (std::runtime_error, "Unknown message type: " << type);
            break;
    }

    return msg;
}
