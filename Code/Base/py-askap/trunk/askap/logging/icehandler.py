import sys
from askap.logging import Handler
from askap import get_config

import Ice, IceStorm
# pylint: disable-msg=W0611
import LoggingService_ice
from askap.logging.interfaces import ILoggerPrx, LogEvent

class IceHandler(Handler):
    """
    A handler class which writes logging records, appropriately formatted,
    to a stream. Note that this class does not close the stream, as
    sys.stdout or sys.stderr may be used.
    """
    def __init__(self):
        """
        Initialize the handler.
        """
        Handler.__init__(self)
        self._setup_iceprxy()
        self.formatter = None

    def _setup_iceprxy(self):


        initData = Ice.InitializationData()
        initData.properties = Ice.createProperties(None, 
                                                   initData.properties)
        initData.properties.load(get_config("askap.logging",
                                            "logging_publisher.ice_cfg"))
        
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
        publisher = topic.getPublisher()
        publisher = publisher.ice_oneway()

        self.prxy = ILoggerPrx.uncheckedCast(publisher)
        
        if not self.prxy:
            raise RuntimeError("Invalid proxy")

    def emit(self, record):
        event = LogEvent()
        event.origin = record.name
        event.level = record.levelname
        event.created = record.created
        event.message = record.msg
        self.prxy.send(event)
    
    def close(self):
        if self.ic:
            self.ic.destroy()
        Handler.close(self)


