#!/usr/bin/env python
import sys
import os
import subprocess
import time
import socket
# pylint: disable-msg=E0611
from nose.tools import assert_equals
import IceStorm
import askap.logging
from askap.iceutils import IceSession
# pylint: disable-msg=W0611
from askap.slice import LoggingService
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
                      event.message, event.tag, event.hostname]

class LogSubscriber(object):
    def __init__(self, comm):
        self.ice = comm
        self.manager = IceStorm.TopicManagerPrx.checkedCast(
            self.ice.stringToProxy(
                'IceStorm/TopicManager@IceStorm.TopicManager'
                )
            )

        topicname = "logger"
        try:
            self.topic = self.manager.retrieve(topicname)
        except IceStorm.NoSuchTopic:
            try:
                self.topic = self.manager.create(topicname)
            except IceStorm.TopicExists:
                self.topic = self.manager.retrieve(topicname)
        # defined in config.icegrid
        self.adapter = \
            self.ice.createObjectAdapterWithEndpoints("LoggingServiceAdapter",
                                                      "tcp")

        subscriber = self.adapter.addWithUUID(LoggerImpl()).ice_twoway()

        qos = {'reliability': 'ordered'}
        try:
            self.topic.subscribeAndGetPublisher(qos, subscriber)
        except IceStorm.AlreadySubscribed:
            self.topic.unsubscribe(self.subscriber)
            self.topic.subscribeAndGetPublisher(qos, self.subscriber)
        self.adapter.activate()


class TestIceLogger(object):
    def __init__(self):
        self.subscriber = None
        self.isession = None

    def setup(self):
        os.environ["ICE_CONFIG"] = 'ice.cfg'
        self.isession = IceSession()#cleanup=True)
        try:
            self.isession.add_app("icebox")
            self.isession.start()
            self.subscriber = LogSubscriber(self.isession.communicator)
        except Exception as ex:
            self.isession.terminate()
            raise

    def teardown(self):
        self.isession.terminate()

    def test_info(self):
        # this sends a log message over icestorm to the logger topic
        subprocess.call(['./tIceAppender'], shell=True)
        time.sleep(1)
        assert_equals(last_event[0], log_origin)
        assert_equals(last_event[-3], log_msg)
        assert_equals(last_event[-1], socket.gethostname())
