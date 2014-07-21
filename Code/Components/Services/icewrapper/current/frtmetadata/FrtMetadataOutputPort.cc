/// @file FrtMetadataOutputPort.cc
///
/// @details
/// This file has been converted from Ben's MetadataOutputPort and is very similar but
/// just works with a different type (simple map of ints instead of TosMetadata 
/// object). Perhaps we can refactor the class hierarchy to avoid duplication of
/// code. This class is intended to be used in communication with the utility
/// controlling the BETA fringe rotator and/or DRx-based delay tracking.
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
/// @author Max Voronkov <Max.Voronkov@csiro.au>

// Include own header file first
#include "FrtMetadataOutputPort.h"

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
#include "tosmetadata/TypedValueMapMapper.h"

// Using
using namespace askap;
using namespace askap::cp::icewrapper;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;

ASKAP_LOGGER(logger, ".FrtMetadataOutputPort");

FrtMetadataOutputPort::FrtMetadataOutputPort(const std::string& locatorHost,
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

FrtMetadataOutputPort::~FrtMetadataOutputPort()
{
    itsOutputPort->detach();
}

/// @brief Send a TypedValueMap message via this port.
///
/// @param[in] message the message to send.
void FrtMetadataOutputPort::send(const std::map<std::string, int>& message)
{
    TypedValueMap mapmessage; 
    TypedValueMapMapper mapper(mapmessage);
    std::vector<casa::String> fields;
    fields.reserve(message.size());
    for (std::map<std::string, int>::const_iterator ci = message.begin(); ci != message.end(); ++ci) {
         mapper.setInt(ci->first, ci->second);
         fields.push_back(ci->first);
    }
    mapper.setStringSeq("fields_list",fields);

    itsProxy->publish(mapmessage);
}

