/// @file MetadataOutputPort.cc
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
#include "MetadataOutputPort.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Ice/Ice.h"
#include "boost/shared_ptr.hpp"

// CP Ice interfaces
#include "TypedValues.h"

// Local package includes
#include "iceutils/CommunicatorConfig.h"
#include "iceutils/CommunicatorFactory.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;

ASKAP_LOGGER(logger, ".MetadataOutputPort");

MetadataOutputPort::MetadataOutputPort(const std::string& locatorHost,
        const std::string& locatorPort,
        const std::string& topicManager,
        const std::string& topic)
{
    CommunicatorConfig config(locatorHost, locatorPort);
    CommunicatorFactory commFactory;
    Ice::CommunicatorPtr comm = commFactory.createCommunicator(config);

    itsOutputPort.reset(new OutputPortType(comm));
    itsOutputPort->attach(topic, topicManager);
    itsProxy = itsOutputPort->getProxy();
    ASKAPCHECK(itsProxy, "Topic proxy was not initialised");
}

MetadataOutputPort::~MetadataOutputPort()
{
    itsOutputPort->detach();
}

void MetadataOutputPort::send(const askap::interfaces::TimeTaggedTypedValueMap& message)
{
    itsProxy->publish(message);
}
