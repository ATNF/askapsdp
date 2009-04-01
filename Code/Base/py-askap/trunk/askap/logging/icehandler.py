import sys
from askap.logging import Handler
from askap import get_config

import Ice
Ice.loadSlice(get_config("askap.logging", "LoggingService.ice"))
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
                                            "logging_client.ice_cfg"))
        
        self.ic = Ice.initialize(sys.argv, initData)
        self.prxy = ILoggerPrx.\
            checkedCast(self.ic.propertyToProxy("ILogger.Proxy"))
        
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


