/// @file IceAppender.cc
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
#include <iceappender/IceAppender.h>

// System includes
#include <iostream>
#include <string>
#include <sstream>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <log4cxx/helpers/stringhelper.h>

// Local package includes
#include <iceappender/LoggingService.h>

// Using
using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace askap;
using namespace askap::interfaces::logging;

IMPLEMENT_LOG4CXX_OBJECT(IceAppender)

IceAppender::IceAppender()
{
}

IceAppender::~IceAppender()
{
    if (itsIceComm) {
        itsIceComm->shutdown();
        itsIceComm->waitForShutdown();
    }
}

void IceAppender::append(const spi::LoggingEventPtr& event, Pool& /*p*/)
{
    if (itsIceComm && itsIceComm->isShutdown()) {
        std::cerr << "Ice is shutdown, cannot send log message" << std::endl;
        return;
    }

    if (itsLogService) {
        // Create the payload
        ILogEvent iceevent;
        iceevent.origin = event->getLoggerName();

        // The ASKAPsoft log archiver interface expects Unix time in seconds
        // (the parameter is a double precision float) where log4cxx returns
        // microseconds.
        iceevent.created = event->getTimeStamp() / 1000.0 / 1000.0;

        iceevent.level = event->getLevel()->toString();
        iceevent.message = event->getMessage();

        // Send
        itsLogService->send(iceevent);
    }
}

void IceAppender::close()
{
    if (this->closed)
    {
        return;
    }

    this->closed = true;
}

bool IceAppender::isClosed() const
{
    return closed;
}

bool IceAppender::requiresLayout() const
{
    return false;
}

void IceAppender::setOption(const LogString& option, const LogString& value)
{
    if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOCATOR_HOST"), LOG4CXX_STR("locator_host"))) {
        itsLocatorHost = value;
    } else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOCATOR_PORT"), LOG4CXX_STR("locator_port"))) {
        itsLocatorPort = value;
    } else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("TOPIC"), LOG4CXX_STR("topic"))) {
        itsLoggingTopic = value;
    } else {
        AppenderSkeleton::setOption(option, value);
    }
}

void IceAppender::activateOptions(log4cxx::helpers::Pool& pool)
{
    // First ensure host, port and topic are set
    if (!verifyOptions()) {
        return;
    }

    // Initialize Ice
    // Get the initialized property set.
    Ice::PropertiesPtr props = Ice::createProperties();

    // Syntax example for the Ice.Default.Locator property:
    // "IceGrid/Locator:tcp -h localhost -p 4061"
    std::stringstream ss;
    ss << "IceGrid/Locator:tcp -h " << itsLocatorHost << " -p " << itsLocatorPort;
    props->setProperty("Ice.Default.Locator", ss.str());

    // Initialize a communicator with these properties.
    Ice::InitializationData id;
    id.properties = props;
    itsIceComm = Ice::initialize(id);

    if (!itsIceComm) {
        ASKAPTHROW (std::runtime_error, "Ice::CommunicatorPtr not initialised. Terminating.");
    }

    // Obtain the topic or create
    IceStorm::TopicManagerPrx topicManager;
    try {
        Ice::ObjectPrx obj = itsIceComm->stringToProxy("IceStorm/TopicManager");
        topicManager = IceStorm::TopicManagerPrx::checkedCast(obj);
    } catch (Ice::ConnectionRefusedException) {
        std::cerr << "Ice connection refused, messages will not be send to the log server" << std::endl;

        // Just return, treat this as non-fatal for the app, even though it
        // is fatal for logging via Ice.
        return;
    }

    IceStorm::TopicPrx topic;
    try {
        topic = topicManager->retrieve(itsLoggingTopic);
    }
    catch (const IceStorm::NoSuchTopic&) {
        topic = topicManager->create(itsLoggingTopic);
    }

    Ice::ObjectPrx pub = topic->getPublisher()->ice_oneway();
    itsLogService = ILoggerPrx::uncheckedCast(pub);
}

bool IceAppender::verifyOptions() const
{
    const std::string error = "IceAppender: Cannot initialise - ";

    if (itsLocatorHost == "") {
        std::cerr << error << "locator host not specified" << std::endl;
        return false;
    } else if (itsLocatorPort == "") {
        std::cerr << error << "locator port not specified" << std::endl;
        return false;
    } else if (itsLoggingTopic == "") {
        std::cerr << error << "logging topic not specified" << std::endl;
        return false;
    } else {
        return true;
    }
}
