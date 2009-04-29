#!/usr/bin/env python
import signal
import sys,os
import subprocess
import initenv
import time
from nose.tools import assert_equal
import Ice
import IceStorm
from askap import get_config
from askap import logging

from askap.logging import LoggingService_ice

# ice doesn't agree with pylint
# pylint: disable-msg=E0611
from askap.logging.interfaces import ILogger


ICEBIN="${ASKAP_ROOT}/3rdParty/Ice/tags/Ice-3.3.0/install/bin/"

LASTEVENT = []


class LoggerImpl(ILogger):
    # pylint: disable-msg=W0613
    def send(self, event, current=None):
        global LASTEVENT
        LASTEVENT = [event.origin, event.level, event.created, event.message]


class LogSubscriber(object):
    def __init__(self):
        initData = Ice.InitializationData()
        initData.properties = Ice.createProperties(None, 
                                                   initData.properties)
        initData.properties.load(get_config("askap.logging",
                                            "logging_server.ice_cfg"))
        
        self.ic = Ice.initialize(sys.argv, initData)
        self.manager = IceStorm.TopicManagerPrx.\
            checkedCast(self.ic.propertyToProxy('TopicManager.Proxy'))

        topicname = "logger"
        try:
            topic = self.manager.retrieve(topicname)
        except IceStorm.NoSuchTopic, e:
            try:
                topic = self.manager.create(topicname)
            except IceStorm.TopicExists, ex:
                print "temporary error. try again"
                raise
        logadapter = self.ic.\
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

class TestIceLogger(object):
    def setup(self):
        # Initalise icebox, create db directory, write config files....
        if not os.path.exists("db"):
            os.mkdir("db")
        self.iceboxcfg = file("config.icebox", "w")
        self.iceboxcfg.write("""IceBox.ServiceManager.Endpoints=tcp -p 9998
IceBox.Service.IceStorm=IceStormService,33:createIceStorm --Ice.Config=config.service
""")
        self.iceboxcfg.close()
        self.configservice = file("config.service", "w")
        self.configservice.write("""IceStorm.InstanceName=LoggerIceStorm
IceStorm.TopicManager.Endpoints=default -p 10009
IceStorm.Publish.Endpoints=tcp -p 10001:udp -p 10001
IceStorm.Trace.TopicManager=2
Freeze.DbEnv.IceStorm.DbHome=db
""")
        self.configservice.close()
        self.icebox = \
            subprocess.Popen("%s/icebox --Ice.Config=%s" % (ICEBIN,
                                                            self.iceboxcfg.name),
                             shell=1,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        # wait for icebox to be initilised, Could use thred to query stdout for
        # ready status, but this is easier.
        time.sleep(2)
        self.logger = logging.getLogger(__name__)
        # set up IceHandler
        try:
            hand = logging.IceHandler()
            self.logger.addHandler(hand)
            self.subscriber = LogSubscriber()
        except:
            self.cleanup()
            raise
        
    def cleanup(self):
        #clean up config files db and terminate icebox
        import shutil
        os.kill(self.icebox.pid, signal.SIGKILL)
        for f in [self.configservice.name, self.iceboxcfg.name]:
            if os.path.exists(f):
                os.remove(f)
        if os.path.exists("db"):
            shutil.rmtree("db", ignore_errors=True)
        
        
    def test_info(self):
        self.logger.setLevel(logging.INFO)
        msg = "Log Test"
        self.logger.info(msg)
        time.sleep(0.1)
        assert_equal(LASTEVENT[0], __name__)
        assert_equal(LASTEVENT[-1], msg)
            
    def teardown(self):
        self.cleanup()        
    