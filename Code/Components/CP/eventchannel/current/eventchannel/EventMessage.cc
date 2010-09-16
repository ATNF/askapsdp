/// @file EventMessage.cc
///
/// @copyright (c) 2010 CSIRO
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
#include "EventMessage.h"

// Include package level header file
#include "askap_eventchannel.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "cms/MapMessage.h"

ASKAP_LOGGER(logger, ".EventMessage");

using namespace askap;
using namespace askap::cp;
using namespace askap::cp::eventchannel;

EventMessage::EventMessage(cms::MapMessage* message)
        : itsMapMessage(message)
{
    // TODO: Investigate why when trying to set properties a
    // MessageNotWriteableException is thrown. This message should
    // be writable.
    //itsMapMessage->setStringProperty("EventMessageType", "EventMessage");
}

EventMessage::~EventMessage()
{
    itsMapMessage.reset();
}

cms::Message* EventMessage::getCmsMessage(void)
{
    return itsMapMessage.get();
}

std::vector<std::string> EventMessage::getMapNames(void)
{
    return itsMapMessage->getMapNames();
}

bool EventMessage::itemExists(const std::string& key)
{
    return itsMapMessage->itemExists(key);
}

//
// Setters
//

void EventMessage::setBoolean(const std::string& key, const bool val)
{
    itsMapMessage->setBoolean(key, val);
}

void EventMessage::setChar(const std::string& key, const char val)
{
    itsMapMessage->setChar(key, val);
}

void EventMessage::setBytes(const std::string& key, const std::vector<unsigned char> &val)
{
    itsMapMessage->setBytes(key, val);
}

void EventMessage::setShort(const std::string& key, const short val)
{
    itsMapMessage->setShort(key, val);
}

void EventMessage::setInt(const std::string& key, const int val)
{
    itsMapMessage->setInt(key, val);
}

void EventMessage::setLong(const std::string& key, const  long val)
{
    itsMapMessage->setLong(key, val);
}

void EventMessage::setFloat(const std::string& key, const float val)
{
    itsMapMessage->setFloat(key, val);
}

void EventMessage::setDouble(const std::string& key, const double val)
{
    itsMapMessage->setDouble(key, val);
}

void EventMessage::setString(const std::string& key, const std::string& val)
{
    itsMapMessage->setString(key, val);
}

//
// Getters
//

bool EventMessage::getBoolean(const std::string& key)
{
    return itsMapMessage->getBoolean(key);
}

char EventMessage::getChar(const std::string& key)
{
    return itsMapMessage->getChar(key);
}

std::vector<unsigned char> EventMessage::getBytes(const std::string& key)
{
    return itsMapMessage->getBytes(key);
}

short EventMessage::getShort(const std::string& key)
{
    return itsMapMessage->getShort(key);
}

int EventMessage::getInt(const std::string& key)
{
    return itsMapMessage->getInt(key);
}

long EventMessage::getLong(const std::string& key)
{
    return itsMapMessage->getLong(key);
}

float EventMessage::getFloat(const std::string& key)
{
    return itsMapMessage->getFloat(key);
}

double EventMessage::getDouble(const std::string& key)
{
    return itsMapMessage->getDouble(key);
}

std::string EventMessage::getString(const std::string& key)
{
    return itsMapMessage->getString(key);
}
