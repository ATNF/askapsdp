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
#include <unistd.h>

// ASKAPsoft includes
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <log4cxx/helpers/stringhelper.h>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/circular_buffer.hpp>

// Ice interfaces includes
#include "LoggingService.h"

// Using
using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace askap;
using namespace std;

std::map<LevelPtr, askap::interfaces::logging::LogLevel> IceAppender::theirLevelMap;

IMPLEMENT_LOG4CXX_OBJECT(IceAppender)

IceAppender::IceAppender() : itsBuffer(DEFAULT_BUF_CAPACITY)
{
    if (theirLevelMap.size() == 0) {
        theirLevelMap[Level::getTrace()] = askap::interfaces::logging::TRACE;
        theirLevelMap[Level::getDebug()] = askap::interfaces::logging::DEBUG;
        theirLevelMap[Level::getInfo()] = askap::interfaces::logging::INFO;
        theirLevelMap[Level::getWarn()] = askap::interfaces::logging::WARN;
        theirLevelMap[Level::getError()] = askap::interfaces::logging::ERROR;
        theirLevelMap[Level::getFatal()] = askap::interfaces::logging::FATAL;
    }
    itsLogHost = getHostName(true);

    // Set a default TopicManager identity (it can optionally be passed as
    // a parameter in the log config which will override this)
    itsTopicManager = "IceStorm/TopicManager@IceStorm.TopicManager";
}

IceAppender::~IceAppender()
{
    if (itsThread.get()) {
        itsThread->interrupt();
        itsThread->join();
    }

    if (itsIceComm) {
        itsIceComm->shutdown();
        itsIceComm->waitForShutdown();
        itsIceComm->destroy();
        itsIceComm = 0;
    }
}

void IceAppender::append(const spi::LoggingEventPtr& event, Pool& /*pool*/)
{
    askap::interfaces::logging::ILogEvent iceevent;
    iceevent.origin = event->getLoggerName();

    // The ASKAPsoft log archiver interface expects Unix (posix) time in
    // seconds (the parameter is a double precision float) where log4cxx
    // returns microseconds.
    iceevent.created = event->getTimeStamp() / 1000.0 / 1000.0;
    iceevent.level = theirLevelMap[event->getLevel()];
    iceevent.message = event->getMessage();
    iceevent.hostname = itsLogHost;
    iceevent.tag = itsTag;

    // Enqueue for asynchronous sending
    boost::mutex::scoped_lock lock(itsMutex);
    itsBuffer.push_back(iceevent);

    // Notify any waiters
    lock.unlock();
    itsCondVar.notify_all();
}

void IceAppender::close()
{
    if (this->closed) {
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
    } else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("TOPIC_MANAGER"), LOG4CXX_STR("topic_manager"))) {
        itsTopicManager = value;
    } else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("TAG"), LOG4CXX_STR("tag"))) {
        itsTag = value;
    } else {
        AppenderSkeleton::setOption(option, value);
    }
}

void IceAppender::activateOptions(log4cxx::helpers::Pool& /*pool*/)
{
    itsThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&IceAppender::run, this)));
}

bool IceAppender::verifyOptions() const
{
    const string error = "IceAppender: Cannot initialise - ";

    if (itsLocatorHost == "") {
        cerr << error << "locator host not specified" << endl;
        return false;
    } else if (itsLocatorPort == "") {
        cerr << error << "locator port not specified" << endl;
        return false;
    } else if (itsLoggingTopic == "") {
        cerr << error << "logging topic not specified" << endl;
        return false;
    } else {
        return true;
    }
}

std::string IceAppender::getHostName(bool full)
{
    const int MAX_HOSTNAME_LEN = 256;
    char hname[MAX_HOSTNAME_LEN];
    hname[MAX_HOSTNAME_LEN - 1] = '\0';

    if (gethostname(hname, MAX_HOSTNAME_LEN - 1) != 0) {
        return string("localhost");
    }

    string hostname(hname);

    if (!full) {
        string::size_type dotloc = hostname.find_first_of(".");

        if (dotloc != hostname.npos) {
            return hostname.substr(0, dotloc);
        }
    }

    return hostname;
}

void IceAppender::connect(void)
{
    // First ensure host, port and topic are set
    if (!verifyOptions()) {
        return;
    }

    // Initialize Ice
    if (!itsIceComm) {
        // Get the initialized property set.
        Ice::PropertiesPtr props = Ice::createProperties();

        // Syntax example for the Ice.Default.Locator property:
        // "IceGrid/Locator:tcp -h localhost -p 4061"
        stringstream ss;
        ss << "IceGrid/Locator:tcp -h " << itsLocatorHost << " -p " << itsLocatorPort;
        props->setProperty("Ice.Default.Locator", ss.str());

        // Initialize a communicator with these properties.
        Ice::InitializationData id;
        id.properties = props;
        itsIceComm = Ice::initialize(id);
        if (!itsIceComm) {
            cerr << "Could not connect init Ice Communicator. Will attempt send later" << endl;
            // Throttle retry
            const unsigned int sleeptime = DEFAULT_RETRY_INTERVAL;
            boost::this_thread::sleep_for(boost::chrono::seconds(sleeptime));
            return;
        }
    }

    // Obtain a proxy to the topic manager
    IceStorm::TopicManagerPrx topicManager;
    try {
        Ice::ObjectPrx obj = itsIceComm->stringToProxy(itsTopicManager);
        topicManager = IceStorm::TopicManagerPrx::checkedCast(obj);
    } catch (Ice::Exception &e) {
        cerr << "Could not connect to logger topic. Will attempt send later. Error was:" << endl;
        cerr << "Exception: " << e.what() << endl;
        cerr << "Topic Manager Identity: " << itsTopicManager << endl;

        // Throttle retry
        const unsigned int sleeptime = DEFAULT_RETRY_INTERVAL;
        boost::this_thread::sleep_for(boost::chrono::seconds(sleeptime));
        return;
    }

    // Obtain the topic or create
    IceStorm::TopicPrx topic;
    try {
        topic = topicManager->retrieve(itsLoggingTopic);
    } catch (const IceStorm::NoSuchTopic&) {
        try {
            topic = topicManager->create(itsLoggingTopic);
        } catch  (const IceStorm::TopicExists&) {
            try {
                topic = topicManager->retrieve(itsLoggingTopic);
            } catch  (const IceStorm::NoSuchTopic&) {
                cerr << "Topic creation/retrieval failed after two attempts" << endl;
                return;
            }
        }
    }

    Ice::ObjectPrx pub = topic->getPublisher()->ice_twoway();
    itsLogService = askap::interfaces::logging::ILoggerPrx::uncheckedCast(pub);
}

void IceAppender::run(void)
{
    while (!(boost::this_thread::interruption_requested())) {
        boost::mutex::scoped_lock lock(itsMutex);

        // Wait if buffer is empty
        while (itsBuffer.empty()) {
            itsCondVar.wait(lock);
            if (boost::this_thread::interruption_requested()) return;
        }

        // If not connected to IceStorm topic, connect
        if (!itsLogService) {
            lock.unlock();
            connect();
            lock.lock();
        }

        // Send
        if (itsLogService) {
            try {
                // Pop an event
                const askap::interfaces::logging::ILogEvent event = itsBuffer.front();
                itsBuffer.pop_front();
                lock.unlock();

                // Send
                itsLogService->send(event);
            } catch (Ice::Exception &e) {
                // This event is lost if the send fails. Could put it back
                // on the buffer, but would need to be careful to put it back
                // in the right position. It would be easy to think we can hold
                // the lock for th duration of the "send" so the buffer is not
                // modified (and we could just push_front), but holding the lock
                // during "send" would unnecessarily block the append() call,
                // don't do that!!
            }
        }
    }
}
