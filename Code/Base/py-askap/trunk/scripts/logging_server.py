#!/usr/bin/env python

import sys
from askap import get_config
from askap.logging.impl import LoggerImpl
import Ice
import IceStorm

class Server(Ice.Application):
    def run(self, args):
        manager = IceStorm.TopicManagerPrx.\
            checkedCast(self.communicator().\
                            propertyToProxy('TopicManager.Proxy'))

        topicname = "logger"
        try:
            topic = manager.retrieve(topicname)
        except IceStorm.NoSuchTopic, e:
            try:
                topic = manager.create(topicname)
            except IceStorm.TopicExists, ex:
                print self.appName() + ": temporary error. try again"
                raise
        
        logadapter = self.communicator().\
            createObjectAdapter("LoggingService.Subscriber")

        subid = Ice.Identity()
        subid.name = Ice.generateUUID()
        subscriber = logadapter.add(LoggerImpl(), subid)
        subscriber = subscriber.ice_oneway()
        qos = {}
        try:
            topic.subscribeAndGetPublisher(qos, subscriber)
        except IceStorm.AlreadySubscribed, ex:
            raise

        logadapter.activate()
        self.shutdownOnInterrupt()
        self.communicator().waitForShutdown()
        #
        # Unsubscribe all subscribed objects.
        #
        topic.unsubscribe(subscriber)
        return 0

sys.stdout.flush()
app = Server()
sys.exit(app.main(sys.argv, get_config("askap.logging",
                                       "logging_server.ice_cfg")))
