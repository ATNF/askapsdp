#!/usr/bin/env python
import sys
import os
import subprocess
import time
from nose.tools import assert_equals
import initenv
import IceStorm
import askap
from askap.iceutils.icegrid import IceGridSession

# pylint: disable-msg=W0611
from askap.slice import LoggingService_ice
# ice doesn't agree with pylint
# pylint: disable-msg=E0611
from askap.interfaces.logging import ILogger

last_event = None

# pylint: disable-msg=W0232
class LoggerImpl(ILogger):
    # pylint: disable-msg=W0613,W0603,R0201
    def send(self, event, current=None):
        global last_event
        last_event = event

class LogSubscriber(object):
    def __init__(self, comm):
        self.ice = comm
        self.manager = IceStorm.TopicManagerPrx.\
            checkedCast(self.ice.stringToProxy('IceStorm/TopicManager'))

        topicname = "logger"
        try:
            self.topic = self.manager.retrieve(topicname)
        except IceStorm.NoSuchTopic:
            try:
                self.topic = self.manager.create(topicname)
            except IceStorm.TopicExists:
                print "temporary error. try again"
                raise
        # defined in config.icegrid
        self.adapter = \
            self.ice.createObjectAdapter("TestAdapter")

        subscriber = self.adapter.addWithUUID(LoggerImpl()).ice_oneway()
        qos = {}
        try:
            self.topic.subscribeAndGetPublisher(qos, subscriber)
        except IceStorm.AlreadySubscribed:
            raise
        self.adapter.activate()

class TestIceAppender(object):
    def __init__(self):
        self.icecfg = 'config.icegrid'
        self.subscriber = None
        self.igsession = None

    def setup(self):
        self.igsession = IceGridSession(self.icecfg)
        self.igsession.startup()
        try:
            self.igsession.add_icestorm()
            self.subscriber = LogSubscriber(self.igsession.communicator)
        except:
            self.igsession.shutdown()
            raise

    def test_java_ice_logger(self):
        # this sends a log message over icestorm to the logger topic
        subprocess.call(['./run_logger.sh'], shell=True)
        time.sleep(1)
        assert(last_event!=None)

    def teardown(self):
        self.igsession.shutdown()
