from askap.logging import Handler, log_debug, getLogger, basicConfig, DEBUG


class ListHandler(Handler):
    def __init__(self, event):
        Handler.__init__(self)
        self.event = event 

    def emit(self, record):
        self.event.append([record.name, record.getMessage()])

# This is passes in as reference as it is a list. Use this to access it globally
event = []
hand = ListHandler(event)
logger = getLogger(__name__)
logger.setLevel(DEBUG)
logger.addHandler(hand)

@log_debug
def debug_me(arg, kwarg=2):
    pass
    
def test_log_debug():
    debugmestr = 'debug_me:  (1, 2) '
    debugmename = "tests.test_logging"
    # call function to generate log message
    debug_me(1,2)
    # check the last generated log event
    assert debugmename == event[-1][0]
    assert debugmestr == event[-1][1]

