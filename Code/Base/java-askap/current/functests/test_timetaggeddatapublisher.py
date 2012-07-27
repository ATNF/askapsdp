#!/usr/bin/env python
import subprocess
import time
import IceStorm
import askap
from askap.iceutils.icegrid import IceGridSession
from askap.slice import TypedValues
from askap.interfaces.datapublisher import ITimeTaggedTypedValueMapPublisher

# Record of the last metadata update received
lastdata = None

# Class for receiving data
class ITimeTaggedTypedValueMapPublisherImpl(ITimeTaggedTypedValueMapPublisher):
    def publish(self, values, current=None):
        global lastdata
        lastdata = values

# Class that sets up and runs the test
class TestTimeTaggedDataPublisher(object):
    def __init__(self):
        self.icecfg = 'config.icegrid'
        self.subscriber = None
        self.igsession = None        
        self.topicname = "TimeTaggedDataPublisher.test"
    
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
                    stringToProxy('IceStorm/TopicManager@IceStorm.TopicManager'))
            try:
                self.topic = self.manager.retrieve(self.topicname)
            except IceStorm.NoSuchTopic:
                self.topic = self.manager.create(self.topicname)
            # defined in config.icegrid
            self.adapter = self.igsession.communicator.\
                createObjectAdapter("TestAdapter")
            subscriber = self.adapter.\
                addWithUUID(ITimeTaggedTypedValueMapPublisherImpl()).ice_oneway()
            qos = {}
            self.topic.subscribeAndGetPublisher(qos, subscriber)
            self.adapter.activate()
        except:
            self.igsession.shutdown()
            raise        
        
    # Test that metadata is received
    def test_data_received(self):
        # Execute process which generates data
        subprocess.call(['./run_ttdp.sh'], shell=True)
        time.sleep(1)
        # Ensure data was received
        thisdata=lastdata
        assert(thisdata!=None)
        assert(thisdata.timestamp!=None)
        assert(len(thisdata.data)>0)
        # Ensure received data is correct format
        for i in thisdata.data:
          assert(isinstance(i, str))
          assert(isinstance(thisdata.data[i], askap.interfaces.TypedValue))
    
    # Called at the end of each test
    def tearDown(self):
        if self.igsession:
            self.igsession.shutdown()
        self.igsession = None
