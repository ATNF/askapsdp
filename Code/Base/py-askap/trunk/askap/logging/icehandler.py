__all__ = ['IceHandler']

import sys
from logging import Handler
from askap import get_config

import Ice, IceStorm
# pylint: disable-msg=W0611
import LoggingService_ice
# pylint: disable-msg=E0611
from askap.logging.interfaces import ILoggerPrx, ILogEvent

class IceHandler(Handler):
    """
    A handler class which writes logging records, appropriately formatted,
    to an ZeroC Ice publisher.
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
        
        # pylint: disable-msg=W0201
        self.ice = Ice.initialize(sys.argv, initData)
        self.manager = IceStorm.TopicManagerPrx.\
            checkedCast(self.ice.propertyToProxy('TopicManager.Proxy'))

        topicname = "logger"
        try:
            topic = self.manager.retrieve(topicname)
        except IceStorm.NoSuchTopic:
            try:
                topic = self.manager.create(topicname)
            except IceStorm.TopicExists:
                print "temporary error. try again"
                raise
        publisher = topic.getPublisher()
        publisher = publisher.ice_oneway()

        self.prxy = ILoggerPrx.uncheckedCast(publisher)
        
        if not self.prxy:
            raise RuntimeError("Invalid proxy")

    def emit(self, record):
        lvlname = record.levelname
        if lvlname == "CRITICAL":
            lvlname = "FATAL"
        event = ILogEvent(record.name, record.created,
                          lvlname, record.msg)
        self.prxy.send(event)
    
    def close(self):
        if self.ice:
            self.ice.destroy()
        Handler.close(self)


