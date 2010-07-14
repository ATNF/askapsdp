#!/usr/bin/env python
import subprocess
import time
import IceStorm
import askap
from askap.iceutils.icegrid import IceGridSession
from askap.slice import TypedValues_ice
from askap.interfaces.datapublisher import ITypedValueMapPublisher

# Record of the last metadata update received
lastdata = None

# Class for receiving data
class ITypedValueMapPublisherImpl(ITypedValueMapPublisher):
    def publish(self, values, current=None):
        global lastdata
        lastdata = values

# Class that sets up and runs the test
class TestDataPublisher(object):
    def __init__(self):
        self.icecfg = 'config.icegrid'
        self.subscriber = None
        self.igsession = None        
        self.topicname = "DataPublisher.test"
    
    # Called at the start of each test
    def setUp(self):
        self.igsession = IceGridSession(self.icecfg)
        self.igsession.startup()
        try:
            # Add the IceStorm application to IceGrid
            self.igsession.add_icestorm()
            
            # Get the Topic and subscribe to updates
            self.manager = IceStorm.TopicManagerPrx.\
                checkedCast(self.igsession.communicator.\
                    stringToProxy('IceStorm/TopicManager'))
            try:
                self.topic = self.manager.retrieve(self.topicname)
            except IceStorm.NoSuchTopic:
                self.topic = self.manager.create(self.topicname)
            # defined in config.icegrid
            self.adapter = self.igsession.communicator.\
                createObjectAdapter("TestAdapter")
            subscriber = self.adapter.\
                addWithUUID(ITypedValueMapPublisherImpl()).ice_oneway()
            qos = {}
            self.topic.subscribeAndGetPublisher(qos, subscriber)
            self.adapter.activate()
        except:
            self.igsession.shutdown()
            raise        
        
    # Test that metadata is received
    def test_data_received(self):
        # Execute process which generates data
        subprocess.call(['./run_dp.sh'], shell=True)
        time.sleep(1)
        # Ensure data was received
        thisdata=lastdata
        assert(thisdata!=None)
        assert(len(thisdata)>0)
        # Ensure received data is correct format
        for i in thisdata:
          assert(isinstance(i, str))
          assert(isinstance(thisdata[i], askap.interfaces.TypedValue))
    
    # Called at the end of each test
    def tearDown(self):
        if self.igsession:
            self.igsession.shutdown()
        self.igsession = None
