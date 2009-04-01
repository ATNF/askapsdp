#!/usr/bin/env python

import sys
from askap import get_config
from askap.logging.impl import LoggerImpl
import Ice

class Server(Ice.Application):
    def run(self, args):
        logadapter = self.communicator().createObjectAdapter("LoggingService")
        logadapter.add(LoggerImpl(), 
                    self.communicator().stringToIdentity("logger"))
        logadapter.activate()
        self.communicator().waitForShutdown()
        return 0

sys.stdout.flush()
app = Server()
sys.exit(app.main(sys.argv, get_config("askap.logging",
                                       "logging_server.ice_cfg")))
