/// @file ingestcontrol.cc
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

// System includes
#include <uuid/uuid.h>
#include <unistd.h>

// ASKAPsoft includes
#include "Common/ParameterSet.h" // LOFAR
#include "Common/Exceptions.h"  // LOFAR
#include "CommandLineParser.h"
#include "boost/scoped_ptr.hpp"

// ActiveMQ C++ interface includes
#include "activemq/library/ActiveMQCPP.h"
#include "activemq/core/ActiveMQConnectionFactory.h"
#include "cms/Connection.h"
#include "cms/Session.h"
#include "cms/Topic.h"
#include "cms/Queue.h"
#include "cms/MessageProducer.h"
#include "cms/MessageConsumer.h"
#include "cms/Message.h"

// Local package includes

// Using
using namespace activemq;
using namespace activemq::core;
using namespace cms;

// Message types
const std::string startReqType = "ingest_start_request";
const std::string startRespType = "ingest_start_response";

const std::string stopReqType = "ingest_stop_request";
const std::string stopRespType = "ingest_stop_response";

const std::string statusReqType = "ingest_status_request";
const std::string statusRespType = "ingest_status_response";

const std::string shutdownReqType = "ingest_shutdown_request";
const std::string shutdownRespType = "ingest_shutdown_response";

/// Print the usage message
/// @param[in] argv0    the program name, typically from argv[0]
void usage(const std::string argv0)
{
        std::cerr << "Usage: " << argv0 << "-brokerURI <URI> -topicURI <URI> -command <command> [options]" << std::endl;
        std::cerr << "  -brokerURI <URI> \tThe URI of the message queue broker" << std::endl;
        std::cerr << "  -topicURI <URI>  \tThe topic/queue name this program will send commands to" << std::endl;
        std::cerr << "  -command <command> \tEither start, stop or status" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -parset <filename> \tFile containing configuration parameters" << std::endl;
}

/// @return a string containing a unique identifier
std::string getUUID(void)
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
void addParset(MapMessage* message, const std::string& parsetFile)
{
    try {
        LOFAR::ParameterSet parset(parsetFile);

        LOFAR::ParameterSet::const_iterator it = parset.begin();
        while (it != parset.end()) {
            message->setString(it->first, it->second);
            ++it;
        }
    } catch (const LOFAR::APSException& e) {
        std::cerr << "Error: Failed to open parset (" << parsetFile << ")"
            << std::endl;
        exit(1);
    }
}

/// Executes a command, or more specifically sends the control command message
/// and displays the result.
///
/// @param[in] brokerURI the URI for the ActiveMQ broker.
/// @param[in] topicURI  the URI for the destination topic to which the control
///                      commands will be sent.
/// @param[in] requestType  the CMS message type for the request message.
/// @param[in] responseType the CMS message type for the response message.
/// @param[in] parsetFile   the filename for the parameter set. Required only for
///                         an ingest_start_request.
void executeCommand(const std::string& brokerURI,
        const std::string& topicURI,
        const std::string& requestType,
        const std::string& responseType,
        const std::string& parsetFile = "")
{
    // Create a connection
    activemq::library::ActiveMQCPP::initializeLibrary();
    ActiveMQConnectionFactory connectionFactory(brokerURI);
    boost::scoped_ptr<cms::Connection> connection(connectionFactory.createConnection());
    connection->start();

    // Create a session
    boost::scoped_ptr<cms::Session> session(connection->createSession(cms::Session::AUTO_ACKNOWLEDGE));

    // Create a destination and producer
    boost::scoped_ptr<Destination> destination(session->createTopic(topicURI));
    boost::scoped_ptr<MessageProducer> producer(session->createProducer(destination.get()));
    producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

    // Create the request message
    boost::scoped_ptr<MapMessage> message(session->createMapMessage());
    message->setCMSType(requestType);
    const std::string correlationId = getUUID();
    message->setCMSCorrelationID(correlationId);

    // Create a temporary queue for the response to be sent
    boost::scoped_ptr<TemporaryQueue> responseQueue(session->createTemporaryQueue());
    boost::scoped_ptr<MessageConsumer> responseConsumer(session->createConsumer(responseQueue.get()));
    message->setCMSReplyTo(responseQueue.get());

    if (requestType == startReqType) {
        addParset(message.get(), parsetFile);
    }

    producer->send(message.get());

    // Wait for a response
    const int timeout = 10 * 1000; // in milliseconds
    boost::scoped_ptr<Message> response(responseConsumer->receive(timeout));
    if (!response.get()) {
        std::cout << "No response received, giving up waiting" << std::endl;
        return;
    }

    // Cast to a MapMessage and process it
    const MapMessage* mapMessage = dynamic_cast<const MapMessage*>(response.get());
    if (!mapMessage || mapMessage->getCMSType() != responseType) {
        std::cerr << "Message of unexpected type received instead of status response" << std::endl;
        return;
    }
    if (mapMessage->getCMSCorrelationID() != correlationId) {
        std::cerr << "Message of unexpected correlation ID received" << std::endl;
        return;
    }

    std::cout << "Response: " << mapMessage->getString("return") << std::endl;

    //
    // Cleanup
    //
    responseConsumer->close();
    producer->close();
    session->close();
    connection->close();
    activemq::library::ActiveMQCPP::shutdownLibrary();
}

int main(int argc, char *argv[])
{
    try {
        // Setup command line parameter
        cmdlineparser::Parser parser;
        cmdlineparser::FlaggedParameter<std::string> brokerPar("-brokerURI");
        cmdlineparser::FlaggedParameter<std::string> topicPar("-topicURI");
        cmdlineparser::FlaggedParameter<std::string> cmdPar("-command");
        cmdlineparser::FlaggedParameter<std::string> parsetPar("-parset", "");

        parser.add(brokerPar, cmdlineparser::Parser::throw_exception);
        parser.add(topicPar, cmdlineparser::Parser::throw_exception);
        parser.add(cmdPar, cmdlineparser::Parser::throw_exception);
        parser.add(parsetPar, cmdlineparser::Parser::return_default);

        parser.process(argc, const_cast<char**> (argv));

        const std::string brokerURI = brokerPar;
        const std::string topicURI = topicPar;
        const std::string command = cmdPar;
        const std::string parsetFile = parsetPar;


        if (command == "start") {
            if (parsetFile == "") {
                std::cerr << "Error: Must specify a parset for start command"
                    << std::endl;
                return 1;
            }
            executeCommand(brokerURI, topicURI, startReqType, startRespType, parsetFile);
        } else if (command == "stop") {
            executeCommand(brokerURI, topicURI, stopReqType, stopRespType);
        } else if (command == "status") {
            executeCommand(brokerURI, topicURI, statusReqType, statusRespType);
        } else if (command == "shutdown") {
            executeCommand(brokerURI, topicURI, shutdownReqType, shutdownRespType);
        } else {
            std::cerr << "Unknown command. Valid commands are "
                << "\"start\", \"stop\", \"status\", and \"shutdown\"" << std::endl;
        }

    } catch (const cmdlineparser::XParser& e) {
        usage(argv[0]);
        return 1;
    } catch(const CMSException& e) {
        e.printStackTrace();
        return 1;
    }

    return 0;
}
