/// @file MetadataOutPort.cc
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
#include "MetadataOutPort.h"

// Include package level header file
#include "askap_bridges.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "boost/scoped_ptr.hpp"
#include "activemq/core/ActiveMQConnectionFactory.h"
#include "activemq/library/ActiveMQCPP.h"
#include "cms/Session.h"

// Using
using namespace askap::cp;
using namespace activemq;
using namespace activemq::core;
using namespace cms;

ASKAP_LOGGER(logger, ".MetadataOutPort");

MetadataOutPort::MetadataOutPort(const std::string brokerURI, const std::string destURI)
{
    activemq::library::ActiveMQCPP::initializeLibrary();

    // Create a ConnectionFactory
    boost::scoped_ptr<ActiveMQConnectionFactory> connectionFactory(
            new ActiveMQConnectionFactory( brokerURI ) );

    // Create a Connection
    try {
        boost::scoped_ptr<cms::Connection> connection(
                connectionFactory->createConnection());
        connection->start();
        // Create a Session
        boost::scoped_ptr<cms::Session> session(
                connection->createSession(cms::Session::AUTO_ACKNOWLEDGE));

        // Create the destination (Topic)
        boost::scoped_ptr<cms::Destination> destination(
                session->createTopic(destURI));

        // Create a MessageProducer from the Session to the Topic or Queue
        itsProducer.reset(session->createProducer(destination.get()));
        itsProducer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
    } catch(CMSException& e) {
        e.printStackTrace();
        throw e;
    }

}

MetadataOutPort::~MetadataOutPort()
{
    itsProducer.reset();
    activemq::library::ActiveMQCPP::shutdownLibrary();
}

void MetadataOutPort::send(cms::Message* message)
{
    itsProducer->send(message);
}
