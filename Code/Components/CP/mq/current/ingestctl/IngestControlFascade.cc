/// @file IngestControlFascade.cc
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
#include "IngestControlFascade.h"

// Include package level header file
#include <askap_mq.h>

// System includes
#include <uuid/uuid.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"    // LOFAR
#include "Common/Exceptions.h"      // LOFAR
#include "activemq/core/ActiveMQConnectionFactory.h"
#include "activemq/library/ActiveMQCPP.h"
#include "cms/Topic.h"
#include "cms/Queue.h"
#include "cms/TemporaryQueue.h"
#include "cms/MessageProducer.h"
#include "cms/MessageConsumer.h"
#include "cms/Message.h"
#include "cms/MapMessage.h"

// Local package includes
#include "mqutils/MQSession.h"

ASKAP_LOGGER(logger, ".IngestControlFascade");

using namespace askap;
using namespace askap::cp;
using namespace activemq;
using namespace activemq::core;
using namespace cms;

// Message types
static const std::string startReqType = "ingest_start_request";
static const std::string startRespType = "ingest_start_response";

static const std::string abortReqType = "ingest_abort_request";
static const std::string abortRespType = "ingest_abort_response";

static const std::string statusReqType = "ingest_status_request";
static const std::string statusRespType = "ingest_status_response";

static const std::string shutdownReqType = "ingest_shutdown_request";
static const std::string shutdownRespType = "ingest_shutdown_response";

/// @brief Constructor
IngestControlFascade::IngestControlFascade(
        const std::string& brokerURI,
        const std::string& destURI)
{
    // Create session
    try {
        itsMQSession.reset(new MQSession(brokerURI));
    } catch (const cms::CMSException& e) {
        ASKAPTHROW(AskapError, "Error creating MQ connection/session" <<
                e.getMessage());
    }

    // Create a destination and producer
    try {
        itsDestination.reset(itsMQSession->get().createTopic(destURI));
        itsProducer.reset(itsMQSession->get().createProducer(itsDestination.get()));
        itsProducer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
    } catch (const cms::CMSException& e) {
        ASKAPTHROW(AskapError, "Error creating MQ destination/producer" <<
                e.getMessage());
    }
}

/// @brief Destructor
IngestControlFascade::~IngestControlFascade()
{
    itsProducer.reset();
    itsDestination.reset();
}

void IngestControlFascade::start(const LOFAR::ParameterSet& parset)
{
    boost::scoped_ptr<MapMessage> request(itsMQSession->get().createMapMessage());
    request->setCMSType(startReqType);
    addParset(request.get(), parset);
    boost::scoped_ptr<MapMessage> response(sendRequest(request.get()));
}

void IngestControlFascade::abort(void)
{
    boost::scoped_ptr<MapMessage> request(itsMQSession->get().createMapMessage());
    request->setCMSType(abortReqType);
    boost::scoped_ptr<MapMessage> response(sendRequest(request.get()));
}

IngestControlFascade::PipelineState IngestControlFascade::getState(void)
{
    boost::scoped_ptr<MapMessage> request(itsMQSession->get().createMapMessage());
    request->setCMSType(statusReqType);
    boost::scoped_ptr<MapMessage> response(sendRequest(request.get()));
    const std::string rv = response->getString("return");

    if (rv == "Idle") {
        return IDLE;
    } else if (rv == "Starting") {
        return STARTING;
    } else if (rv == "Running") {
        return RUNNING;
    } else if (rv == "Shuting Down") {
        return SHUTTING_DOWN;
    } else {
        ASKAPTHROW(AskapError, "Unhandled PipelineState");
    }

}

void IngestControlFascade::shutdown(void)
{
    boost::scoped_ptr<MapMessage> request(itsMQSession->get().createMapMessage());
    request->setCMSType(shutdownReqType);
    boost::scoped_ptr<MapMessage> response(sendRequest(request.get()));
}

/// @return a string containing a unique identifier
std::string IngestControlFascade::getUUID(void)
{
    uuid_t uuid;
    uuid_generate(uuid);
    const int MAX_LEN = 38;
    char buf[MAX_LEN];
    uuid_unparse(uuid, buf);
    return std::string(buf);
}

/// Given a filename for a LOFAR style parameter set, and a pointer to a
/// map message, this function reads each key/value pair from the parset
/// and adds it to the map message.
///
/// @param[in]  message the map message to which the key/value pairs will
///             be added
/// @param[in]  parsetFile the filename for the parameter set
void IngestControlFascade::addParset(MapMessage* message, const LOFAR::ParameterSet& parset)
{
    LOFAR::ParameterSet::const_iterator it = parset.begin();
    while (it != parset.end()) {
        message->setString(it->first, it->second);
        ++it;
    }
}

cms::MapMessage* IngestControlFascade::sendRequest(cms::Message* request,
        const int timeout)
{
    // Create a temporary queue and consumer for the response to be sent
    boost::scoped_ptr<TemporaryQueue> responseQueue(itsMQSession->get().createTemporaryQueue());
    boost::scoped_ptr<MessageConsumer> responseConsumer(itsMQSession->get().createConsumer(responseQueue.get()));
    request->setCMSReplyTo(responseQueue.get());

    // Set a UUID for message correlation
    const std::string correlationId = getUUID();
    request->setCMSCorrelationID(correlationId);

    itsProducer->send(request);

    // Wait for a response
    Message* response = responseConsumer->receive(timeout);
    if (!response) {
        ASKAPTHROW(AskapError, "Timeout exceeded waiting for response");
    }

    // Cast to a MapMessage and process it
    if (response->getCMSCorrelationID() != correlationId) {
        ASKAPTHROW(AskapError, "Message of unexpected correlation ID received");
    }

    return dynamic_cast<MapMessage*>(response);
}
