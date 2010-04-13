/// @file CommunicatorConfig.cc
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
#include "CommunicatorConfig.h"

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "Ice/Ice.h"

// Using
using namespace askap::cp;

CommunicatorConfig::CommunicatorConfig(const std::string& locatorHost,
        const std::string& locatorPort)
{
    // Set the locator property
    // Syntax example:
    // IceGrid/Locator:tcp -h localhost -p 4061
    std::ostringstream ss;
    ss << "IceGrid/Locator:tcp -h ";
    ss << locatorHost;
    ss << " -p ";
    ss << locatorPort;
    setProperty("Ice.Default.Locator", ss.str());

    //
    // Plus add some default properties
    //
 
    // Make sure that network and protocol tracing are off by default.
    // This can however be overridden by a call to setProperty()
    setProperty("Ice.Trace.Network", "0");
    setProperty("Ice.Trace.Protocol", "0");

    // Increase maximum message size from 1MB to 128MB
    // This can however be overridden by a call to setProperty()
    setProperty("Ice.MessageSizeMax", "131072");
}

void CommunicatorConfig::setProperty(const std::string& key, const std::string& value)
{
    itsProperties[key] = value;
}

void CommunicatorConfig::removeProperty(const std::string& key)
{
    itsProperties.erase(key);
}

void CommunicatorConfig::setAdapter(const std::string& name, const std::string& endpoints)
{
    // Syntax example:
    // MyAdapterName.AdapterId=MyAdapterName
    // MyAdapterName.Endpoints=tcp
    //std::ostringstream adapterId;
    //adapterId << name << ".AdapterId";
    //setProperty(adapterId.str(), name);

    // NOTE: The creation of the AdapterId is disabled (above) so that Ice
    // creates a unique Id for each instance of an application.
    
    std::ostringstream epprop;
    epprop << name << ".Endpoints";
    setProperty(epprop.str(), "tcp");

}

void CommunicatorConfig::removeAdapter(const std::string& name)
{
    std::ostringstream adapterId;
    adapterId << name << ".AdapterId";
    removeProperty(adapterId.str());

    std::ostringstream epprop;
    epprop << name << ".Endpoints";
    removeProperty(epprop.str());
}

Ice::PropertiesPtr CommunicatorConfig::convertToIceProperties(void) const
{
    Ice::PropertiesPtr props = Ice::createProperties();
    std::map<std::string, std::string>::const_iterator it;
    for (it = itsProperties.begin(); it != itsProperties.end(); it++) {
        props->setProperty(it->first, it->second);
    }

    return props;
}
