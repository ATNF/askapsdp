/// @file ConnectionWrapper.cc
///
/// @copyright (c) 2011 CSIRO
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
#include "ConnectionWrapper.h"

// Include package level header file
#include "askap_channels.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "boost/scoped_ptr.hpp"
#include "activemq/core/ActiveMQConnection.h"
#include "activemq/core/ActiveMQConnectionFactory.h"
#include "cms/Connection.h"
#include "cms/Session.h"
#include "cms/ExceptionListener.h"

ASKAP_LOGGER(logger, ".ConnectionWrapper");

// Using
using namespace std;
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::channels;
using namespace activemq;
using namespace activemq::core;
using namespace cms;

ConnectionWrapper::ConnectionWrapper(const std::string& brokerURI)
{
    ASKAPLOG_DEBUG_STR(logger, "Connecting with URI: " << brokerURI);

    // Create a ConnectionFactory
    boost::scoped_ptr<ActiveMQConnectionFactory> connectionFactory(
        new ActiveMQConnectionFactory(brokerURI));

    try {
        // Create a Connection
        itsConnection.reset(connectionFactory->createConnection());
        //((activemq::core::ActiveMQConnection*)itsConnection.get())->setUseAsyncSend(true);
        itsConnection->start();

        // Create a Session
        itsSession.reset(itsConnection->createSession(cms::Session::AUTO_ACKNOWLEDGE));
    } catch (const cms::CMSException& e) {
        ASKAPLOG_WARN_STR(logger, "Exception connecting to uv-channel: " << e.getMessage());
        ASKAPTHROW(AskapError, e.getMessage());
    }
}

ConnectionWrapper::~ConnectionWrapper()
{
    try {
        ASKAPLOG_DEBUG_STR(logger, "Disconnecting");
        itsConnection->stop();

        // Cleanup session
        itsSession->close();
        itsSession.reset();

        // Clean up connection
        itsConnection->close();
        itsConnection.reset();
    } catch (const cms::CMSException& e) {
        ASKAPLOG_WARN_STR(logger, "Exception caught in ~ConnectionWrapper: "
                << e.getMessage());
    } catch (...) {
        // No exception should escape from destructor
        ASKAPLOG_WARN_STR(logger, "Exception caught in ~ConnectionWrapper");
    }
}

cms::Session* ConnectionWrapper::getSession(void)
{
    return itsSession.get();
}

void ConnectionWrapper::onException(const cms::CMSException& e)
{
    ASKAPLOG_WARN_STR(logger, "Exception on UVChannel: " << e.getMessage());
}
