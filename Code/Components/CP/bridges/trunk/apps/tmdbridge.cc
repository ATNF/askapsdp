/// @file tmdbridge.cc
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
#include <iostream>
#include <string>

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>

// Local package includes
#include "interfaces/TypedValues.h"

// Using
using namespace askap::interfaces::datapublisher;
using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace cms;

class MetadataOutPort {
    public:
        MetadataOutPort(const std::string& locatorHost,
                const std::string& locatorPort,
                const std::string& topicManager,
                const std::string& topic)
        {
            Ice::PropertiesPtr props = Ice::createProperties();

            // Make sure that network and protocol tracing are off.
            props->setProperty("Ice.Trace.Network", "0");
            props->setProperty("Ice.Trace.Protocol", "0");

            // Increase maximum message size from 1MB to 128MB
            props->setProperty("Ice.MessageSizeMax", "131072");

            // Syntax example:
            // IceGrid/Locator:tcp -h localhost -p 4061
            std::ostringstream ss;
            ss << "IceGrid/Locator:tcp -h ";
            ss << locatorHost;
            ss << " -p ";
            ss << locatorPort;
            std::string locatorParam = ss.str();

            props->setProperty("Ice.Default.Locator", locatorParam);

            // Initialize a communicator with these properties.
            Ice::InitializationData id;
            id.properties = props;
            itsComm = Ice::initialize(id);

            // Get the topic for the metadata stream
            itsMetadataStream = ITimeTaggedTypedValueMapPublisherPrx::uncheckedCast(
                    getProxy(topicManager, topic));
        }

        void send(const askap::interfaces::TimeTaggedTypedValueMap& payload)
        {
            itsMetadataStream->publish(payload);
        }


    private:
        Ice::ObjectPrx getProxy(const std::string& topicManager,
                const std::string& topic)
        {
            Ice::ObjectPrx obj = itsComm->stringToProxy(topicManager);
            IceStorm::TopicManagerPrx manager =
                IceStorm::TopicManagerPrx::checkedCast(obj);
            IceStorm::TopicPrx topicPrx;
            try {
                topicPrx = manager->retrieve(topic);
            } catch (const IceStorm::NoSuchTopic&) {
                try {
                    topicPrx = manager->create(topic);
                } catch (const IceStorm::TopicExists&) {
                    // Something eles has since created the topic
                    topicPrx = manager->retrieve(topic);
                }
            }

            return topicPrx->getPublisher()->ice_twoway();

        }

        Ice::CommunicatorPtr itsComm;

        askap::interfaces::datapublisher::ITimeTaggedTypedValueMapPublisherPrx itsMetadataStream;
};

class MetadataInPort : public ExceptionListener,
    public MessageListener,
    public DefaultTransportListener
{
    public:
        MetadataInPort(const std::string brokerURI, const std::string destURI)
        {
            activemq::library::ActiveMQCPP::initializeLibrary();

            // Create a ConnectionFactory
            ActiveMQConnectionFactory* connectionFactory =
                new ActiveMQConnectionFactory( brokerURI );

            // Create a Connection
            Connection* connection;
            connection = connectionFactory->createConnection();
            delete connectionFactory;

            ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
            if( amqConnection != NULL ) {
                amqConnection->addTransportListener( this );
            }

            connection->start();

            connection->setExceptionListener(this);

            Session* session = connection->createSession(Session::AUTO_ACKNOWLEDGE);

            Destination* destination = session->createTopic(destURI);

            // Create a MessageConsumer from the Session to the Topic or Queue
            MessageConsumer* consumer = session->createConsumer( destination );
            consumer->setMessageListener( this );

        }

        ~MetadataInPort()
        {
            activemq::library::ActiveMQCPP::shutdownLibrary();
        }

        virtual void onMessage(const Message* message)
        {
            const MapMessage* mapMessage =
                dynamic_cast<const MapMessage*>(message);

            if(mapMessage != NULL) {
                std::cout << "Got a map message" << std::endl;
            } else {
                std::cerr << "This is not a MapMessage" << std::endl;
            }

        }

        virtual void onException( const CMSException& ex AMQCPP_UNUSED )
        {
            std::cerr << "CMS: Message queue exception" << std::endl;
        }

        virtual void transportInterrupted() {
            std::cerr << "The Connection's Transport has been Interrupted." << std::endl;
        }

        virtual void transportResumed() {
            std::cerr << "The Connection's Transport has been Restored." << std::endl;
        }

    private:
};

int main(int argc, char *argv[])
{
    const int count = 10;

    MetadataOutPort out("localhost", "4061", "IceStorm/TopicManager", "tosmetadata");
    MetadataInPort("failover:(tcp://127.0.0.1:61616)", "tosmetadata");

    for (int i = 0; i < count; ++i) {
        askap::interfaces::TimeTaggedTypedValueMap metadata;
        const long time = 1234567890;
        metadata.timestamp = time;
        metadata.data["time"] = new askap::interfaces::TypedValueLong(askap::interfaces::TypeLong, time);
        out.send(metadata);
    }

    sleep(10);

    return 0;
}
