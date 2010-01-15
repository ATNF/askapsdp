/// @file ActivityDesc.cc
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
#include "ActivityDesc.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"

// Using
using namespace askap;
using namespace askap::cp;

ASKAP_LOGGER(logger, ".ActivityDesc");

ActivityDesc::ActivityDesc()
{
}

ActivityDesc::~ActivityDesc()
{
}

void ActivityDesc::setRuntime(const std::string& runtime)
{
    itsRuntime = runtime;
}

void ActivityDesc::setType(const std::string& type)
{
    itsType = type;
}

void ActivityDesc::setName(const std::string& name)
{
    itsName = name;
}

unsigned int ActivityDesc::addInPortMapping(const std::string& stream)
{
    itsInPorts.push_back(stream);
    return itsInPorts.size() - 1;
}

unsigned int ActivityDesc::addOutPortMapping(const std::string& stream)
{
    itsOutPorts.push_back(stream);
    return itsOutPorts.size() - 1;
}

void ActivityDesc::setParset(const LOFAR::ParameterSet& parset)
{
    itsParset = parset;
}

std::string ActivityDesc::getRuntime(void) const
{
    return itsRuntime;
}

std::string ActivityDesc::getType(void) const
{
    return itsType;
}

std::string ActivityDesc::getName(void) const
{
    return itsName;
}

unsigned int ActivityDesc::getNumInPorts(void) const
{
    return itsInPorts.size();
}

unsigned int ActivityDesc::getNumOutPorts(void) const
{
    return itsOutPorts.size();
}

std::string ActivityDesc::getPortInPortMapping(unsigned int port) const
{
    if (port > itsInPorts.size() - 1) {
        ASKAPTHROW(AskapError, "Invalid port number");
    }
    return itsInPorts.at(port);
}

std::string ActivityDesc::getPortOutPortMapping(unsigned int port) const
{
    if (port > itsOutPorts.size() - 1) {
        ASKAPTHROW(AskapError, "Invalid port number");
    }
    return itsOutPorts.at(port);
}

LOFAR::ParameterSet ActivityDesc::getParset(void) const
{
    return itsParset;
}
