/// @file logsubscribe.cc
///
/// @detail
/// A text based log subscriber. This is a simple standalone utility for
/// displaying log events to stdout.
///
/// @copyright (c) 2013 CSIRO
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

// Package level header file
#include "askap_logappenders.h"

// System includes
#include <string>
#include <iostream>
#include <unistd.h>
#include <ctime>

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "boost/program_options.hpp"

// Ice Interfaces
#include "LoggingService.h"

// Using
using namespace std;
using namespace Ice;
namespace po = boost::program_options;

class EventHandler : public askap::interfaces::logging::ILogger
{
    public:
        virtual void send(const askap::interfaces::logging::ILogEvent& event,
                          const Ice::Current&)
        {
            cout << convertTime(event.created) << " (" << event.hostname << ", "
                << event.origin << ") - " << event.message << endl;
        }

    private:
        // Converts time in seconds to an ASCII representation
        // eg: "Fri Nov  8 17:48:20 2013"
        static std::string convertTime(const double time) {
            time_t tt = static_cast<time_t>(time);
            const char* ascii = asctime(localtime(&tt));
            if (ascii) {
                // Need to remove the '\n'
                string s(ascii);
                return s.substr(0, s.length() - 1);
            } else {
                return string("<Invalid time>");
            }
        }
};

int main(int argc, char *argv[])
{
    // Constants
    const string TOPIC_MANAGER("IceStorm/TopicManager@IceStorm.TopicManager");
    const string DEFAULT_LOCATOR_HOST("localhost");
    const int DEFAULT_PORT_HOST(4061);
    const string DEFAULT_TOPIC("logger");

    // Parse command line parameters
    string locatorHost;
    int locatorPort;
    string topic;
    boost::program_options::options_description options("Program Options");
    options.add_options()
        ("help,?", "help message")
        ("host,h", po::value<string>(&locatorHost)->default_value(DEFAULT_LOCATOR_HOST),
            "Ice locator host")
        ("port,p", po::value<int>(&locatorPort)->default_value(DEFAULT_PORT_HOST),
            "Ice locator port")
        ("topic,t", po::value<string>(&topic)->default_value(DEFAULT_TOPIC),
            "logger topic");

    boost::program_options::variables_map varMap;
    po::store(po::parse_command_line(argc, argv, options), varMap);
    po::notify(varMap);

    if (varMap.count("help") > 0) {
        cerr << options << std::endl;
        exit(EXIT_FAILURE);
    }

    // Configure Properties
    ostringstream ss;
    ss << "IceGrid/Locator:tcp -h ";
    ss << locatorHost;
    ss << " -p ";
    ss << locatorPort;

    Ice::PropertiesPtr props = Ice::createProperties();
    props->setProperty("Ice.Default.Locator", ss.str());
    props->setProperty("Ice.Trace.Network", "0");
    props->setProperty("Ice.Trace.Protocol", "0");
    props->setProperty("Ice.IPv6", "0");
    props->setProperty("Ice.Default.EncodingVersion", "1.0");
    props->setProperty("LogSubscriberAdapterName.Endpoints", "tcp");

    // So logging is serialised through a single thread
    props->setProperty("Ice.ThreadPool.Server.SizeMax", "1");

    cout << "Contacting Locator Host: " << locatorHost << ":" << locatorPort << endl;

    // Create Ice Communicator
    Ice::InitializationData id;
    id.properties = props;
    Ice::CommunicatorPtr comm = Ice::initialize(id);
    if (!comm) {
        cerr << "ERROR: Failed to initialise communicator" << endl;
        exit(EXIT_FAILURE);
    }

    // Create the callback handler
    EventHandler handler;

    // Now subscribe to the topic
    Ice::ObjectPrx obj = comm->stringToProxy(TOPIC_MANAGER);
    IceStorm::TopicManagerPrx topicManagerPrx = IceStorm::TopicManagerPrx::checkedCast(obj);
    Ice::ObjectAdapterPtr adapter = comm->createObjectAdapter("LogSubscriberAdapterName");
    Ice::ObjectPrx proxy = adapter->addWithUUID(&handler)->ice_twoway();

    cout << "Subscribing to topic: " << topic << endl;

    IceStorm::TopicPrx topicPrx;
    try {
        topicPrx = topicManagerPrx->retrieve(topic);
    } catch (const IceStorm::NoSuchTopic&) {
        cerr << "Topic not found, creating." << endl;
        try {
            topicPrx = topicManagerPrx->create(topic);
        } catch (const IceStorm::TopicExists&) {
            // Someone else has already created it
            topicPrx = topicManagerPrx->retrieve(topic);
        }
    }

    adapter->activate();

    IceStorm::QoS qos;
    qos["reliability"] = "ordered";
    topicPrx->subscribeAndGetPublisher(qos, proxy);

    comm->waitForShutdown();

    return 0;
}
