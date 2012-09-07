#!/usr/bin/env python
import os
import subprocess
import time
import IceStorm
import askap
from askap.iceutils import IceSession, get_service_object
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
        self.subscriber = None
        self.isession = None        
        self.topicname = "TimeTaggedDataPublisher.test"
    
    # Called at the start of each test
    def setUp(self):
        os.environ["ICE_CONFIG"] = 'ice.cfg'
        self.isession = IceSession(cleanup=True)
        try:
            # Add the IceStorm application
            self.isession.add_app("icebox")
            self.isession.start()
            # Get the Topic and subscribe to updates
            self.manager = get_service_object(
                self.isession.communicator,
                'IceStorm/TopicManager@IceStorm.TopicManager',
                IceStorm.TopicManagerPrx)
            try:
                self.topic = self.manager.retrieve(self.topicname)
            except IceStorm.NoSuchTopic:
                self.topic = self.manager.create(self.topicname)
            # defined in config.icegrid
            self.adapter = self.isession.communicator.\
                createObjectAdapterWithEndpoints("TestAdapter", "tcp")
            subscriber = self.adapter.\
               addWithUUID(ITimeTaggedTypedValueMapPublisherImpl()).ice_oneway()
            qos = {}
            self.topic.subscribeAndGetPublisher(qos, subscriber)
            self.adapter.activate()
        except:
            self.isession.terminate()
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
        self.isession.terminate()
