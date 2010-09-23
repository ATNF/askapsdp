/// @file IngestController.cc
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
#include "IngestController.h"

// Include package level header file
#include <askap_cpingest.h>

// System includes
#include <string>
#include <vector>
#include <stdexcept>

// ActiveMQ includes
#include "activemq/core/ActiveMQConnectionFactory.h"
#include "activemq/library/ActiveMQCPP.h"
#include "cms/Connection.h"
#include "cms/Session.h"
#include "cms/MessageListener.h"
#include "cms/ExceptionListener.h"
#include "cms/Message.h"
#include "cms/CMSException.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "boost/scoped_ptr.hpp"

// Local package includes
#include "ingestpipeline/IngestPipeline.h"

ASKAP_LOGGER(logger, ".IngestController");

using namespace askap;
using namespace askap::cp::ingest;
using namespace activemq;
using namespace activemq::core;
using namespace cms;

IngestController::IngestController(const std::string& brokerURI, const std::string& topicURI)
    : itsState(IDLE), itsTopicURI(topicURI)
{
    activemq::library::ActiveMQCPP::initializeLibrary();

    // Create a ConnectionFactory
    boost::scoped_ptr<ActiveMQConnectionFactory> connectionFactory(
            new ActiveMQConnectionFactory( brokerURI ) );

    // Create a Connection
    itsConnection.reset(connectionFactory->createConnection());
    itsConnection->start();

    // Create a Session
    itsSession.reset(itsConnection->createSession(cms::Session::AUTO_ACKNOWLEDGE));
}

IngestController::~IngestController()
{
    // Cleanup session
    itsSession->close();
    itsSession.reset();

    // Clean up connection
    itsConnection->close();
    itsConnection.reset();

    // Shutdown the library
    activemq::library::ActiveMQCPP::shutdownLibrary();
}

void IngestController::run(void)
{
    boost::scoped_ptr<Destination> destination(itsSession->createTopic(itsTopicURI));
    boost::scoped_ptr<MessageConsumer> consumer(itsSession->createConsumer(destination.get()));
    consumer->setMessageListener(this);

    //  Main loop
    ASKAPLOG_DEBUG_STR(logger, "Running");
    while (itsState != SHUTTING_DOWN) {
        sleep(1);
        if (itsState == STARTING) {
            try {
                const LOFAR::ParameterSet subset = itsParset.makeSubset("cp.ingest.");
                itsPipeline.reset(new IngestPipeline(subset));
                itsState = RUNNING;
                itsPipeline->start();
            } catch (const askap::AskapError& e) {
                ASKAPLOG_ERROR_STR(logger,
                        "Failed to start IngestPipeline: " << e.what());
            } catch (const std::exception& e) {
                ASKAPLOG_ERROR_STR(logger,
                        "Failed to start IngestPipeline: " << e.what());
            } catch (...) {
                ASKAPLOG_ERROR_STR(logger,
                        "Failed to start IngestPipeline Caught ... ");
            }
            itsPipeline.reset(); // Destroy
            itsParset.clear();
            itsState = IDLE;
        }
    }
}

void IngestController::startCmd(const cms::Message* request)
{
    const std::string responseMsgType = "ingest_start_response";

    // Only transition if the pipeline is idle
    if (itsState != IDLE) {
        sendResponse(request, responseMsgType, "Pipeline is already running");
        return;
    }

    // Populate the parset from the message
    const MapMessage* mapMessage = dynamic_cast<const MapMessage*>(request);
    if (!mapMessage) {
        const std::string error = "Error: Start message not a MapMessage";
        ASKAPLOG_DEBUG_STR(logger, error);
        sendResponse(request, responseMsgType, error);
        return;
    }

    itsParset = buildParset(mapMessage);

    // Change state to starting and let the main thread, the one who called 
    // run() actually start the pipeline
    itsState = STARTING;

    sendResponse(request, responseMsgType, "Start acknowledged");
}

void IngestController::abortCmd(const cms::Message* request)
{
    const std::string responseMsgType = "ingest_abort_response";

    // If already idle
    if (itsState == IDLE) {
        sendResponse(request, responseMsgType, "Pipeline is already idle");
        return;
    }

    itsPipeline->abort();

    sendResponse(request, responseMsgType, "Abort acknowledged");
}

void IngestController::statusCmd(const cms::Message* request)
{
    const std::string responseMsgType = "ingest_status_response";

    std::string result;
    switch (itsState) {
        case IDLE :
            result = "Idle";
            break;
        case STARTING :
            result = "Starting";
            break;
        case RUNNING :
            result = "Running";
            break;
        case SHUTTING_DOWN :
            result = "Shutting down";
            break;
        default:
            ASKAPTHROW(AskapError, "Unhandled PipelineState");
    }
    sendResponse(request, responseMsgType, result);
}

void IngestController::onMessage(const cms::Message* message)
{
    const std::string startMsgType = "ingest_start_request";
    const std::string abortMsgType = "ingest_abort_request";
    const std::string statusMsgType = "ingest_status_request";

    const std::string msgType = message->getCMSType();

    if (msgType == startMsgType) {
        startCmd(message);
    } else  if (msgType == abortMsgType) {
        abortCmd(message);
    } else if (msgType == statusMsgType) {
        statusCmd(message);
    } else {
        ASKAPLOG_DEBUG_STR(logger, "Message of unknown type received ("
                << msgType << ")");
    }
}

void IngestController::onException(const cms::CMSException& ex)
{
    ASKAPLOG_WARN_STR(logger, "Message Queue Exception: " << ex.getMessage());
}

void IngestController::sendResponse(const cms::Message* request,
        const std::string& responseMsgType,
        const std::string& message)
{
    if (!request->getCMSReplyTo()) {
        ASKAPLOG_WARN_STR(logger,
                "Control request has no reply-to set, no reply wil be sent");
    }

    boost::scoped_ptr<MapMessage> response(itsSession->createMapMessage());
    response->setCMSCorrelationID(request->getCMSCorrelationID());
    response->setCMSType(responseMsgType);
    response->setString("return", message);

    boost::scoped_ptr<MessageProducer> producer(itsSession->createProducer(0));
    producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

    producer->send(request->getCMSReplyTo(), response.get());
    producer->close();
}

LOFAR::ParameterSet IngestController::buildParset(const cms::MapMessage* message)
{
    LOFAR::ParameterSet parset;

    std::vector<std::string> mapNames = message->getMapNames();
    std::vector<std::string>::const_iterator it = mapNames.begin();

    while (it != mapNames.end()) {
        parset.add(*it, message->getString(*it));
        ++it;
    }

    return parset;
}
