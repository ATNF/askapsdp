
import datetime
# pylint: disable-msg=W0611
import LoggingService_ice

# ice doesn't agree with pylint
# pylint: disable-msg=E0611
from askap.logging.interfaces import ILogger

# pylint: disable-msg=W0232
class LoggerImpl(ILogger):
    # pylint: disable-msg=W0613,R0201
    def send(self, event, current=None):
        print """Event: %s
  Level:   %s
  Time:    %s
  Message: %s
""" % (event.origin, event.level, 
       datetime.datetime.fromtimestamp(event.created),
       event.message)
