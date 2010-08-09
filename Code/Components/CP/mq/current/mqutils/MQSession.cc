/// @file MQSession.cc
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
#include "MQSession.h"

// Include package level header file
#include <askap_mq.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "activemq/core/ActiveMQConnectionFactory.h"
#include "activemq/library/ActiveMQCPP.h"

ASKAP_LOGGER(logger, ".MQSession");

using namespace askap;
using namespace askap::cp;
using namespace activemq;
using namespace activemq::core;
using namespace cms;

MQSession::MQSession(const std::string& brokerURI)
{
    // Create a ConnectionFactory
    boost::scoped_ptr<ActiveMQConnectionFactory> connectionFactory(
            new ActiveMQConnectionFactory( brokerURI ) );

    // Create a Connection
    itsConnection.reset(connectionFactory->createConnection());
    itsConnection->start();

    // Create a Session
    itsSession.reset(itsConnection->createSession(cms::Session::AUTO_ACKNOWLEDGE));
}

MQSession::~MQSession()
{
    //try {
        // Cleanup session
        itsSession->close();
        itsSession.reset();

        // Clean up connection
        itsConnection->close();
        itsConnection.reset();
    //} catch (const std::exception& e) {
        // No exception should escape from destructor
    //}
}

cms::Session& MQSession::get(void)
{
    return *itsSession;
}
