#!/usr/bin/env python
import sys
import os
import subprocess
import time
import socket
from nose.tools import assert_equals

import initenv

import IceStorm

from askap.iceutils.icegrid import IceGridSession

# pylint: disable-msg=W0611
from askap.slice import LoggingService_ice
# ice doesn't agree with pylint
# pylint: disable-msg=E0611
from askap.interfaces.logging import ILogger

last_event = []
log_msg = "Testing the IceAppender"
log_origin = "MyLogger"

# pylint: disable-msg=W0232
class LoggerImpl(ILogger):
    # pylint: disable-msg=W0613,W0603,R0201
    def send(self, event, current=None):
        global last_event
        last_event = [event.origin, event.level, event.created,
                      event.message, event.hostname]

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
            self.ice.createObjectAdapter("TestLogArchiverAdapter")

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

    def test_info(self):
        # this sends a log message over icestorm to the logger topic
        subprocess.call(['./tIceAppender'], shell=True)
        time.sleep(1)
        assert_equals(last_event[0], log_origin)
        assert_equals(last_event[-2], log_msg)
        assert_equals(last_event[-1], socket.gethostname())

    def teardown(self):
        self.igsession.shutdown()
