/// @file MQSessionSingleton.cc
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
#include "MQSessionSingleton.h"

// Include package level header file
#include <askap_mq.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "activemq/core/ActiveMQConnectionFactory.h"
#include "activemq/library/ActiveMQCPP.h"

ASKAP_LOGGER(logger, ".MQSessionSingleton");

using namespace askap;
using namespace askap::cp;
using namespace activemq;
using namespace activemq::core;
using namespace cms;

MQSessionSingleton* MQSessionSingleton::instance = 0;

MQSessionSingleton::MQSessionSingleton(const std::string& brokerURI)
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

MQSessionSingleton::~MQSessionSingleton()
{
    // Cleanup session
    itsSession->close();
    itsSession.reset();

    // Clean up connection
    itsConnection->close();
    itsConnection->stop();
    itsConnection.reset();

    // Shutdown the library
    activemq::library::ActiveMQCPP::shutdownLibrary();
}

void MQSessionSingleton::initialize(const std::string& brokerURI)
{
    if (instance == 0) {
        instance = new MQSessionSingleton(brokerURI);
    } else {
        ASKAPTHROW(AskapError, "MQSessionSingleton is already initialized");
    }
}

void MQSessionSingleton::shutdown(void)
{
    if (instance) {
        delete instance;
    } else {
        ASKAPTHROW(AskapError, "MQSessionSingleton is not initialized");
    }
}

MQSessionSingleton& MQSessionSingleton::getInstance(void)
{
    if (instance) {
        return *instance;
    } else {
        ASKAPTHROW(AskapError, "MQSessionSingleton is not initialized");
    }
}
