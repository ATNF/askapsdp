from askap.logging import Handler, log_debug, getLogger, DEBUG


class ListHandler(Handler):
    def __init__(self):
        Handler.__init__(self)
        self.event = None

    def emit(self, record):
        self.event = [record.name, record.getMessage()]

# This is passes in as reference as it is a list. Use this to access it globally
#event = None
hand = ListHandler()
logger = getLogger(__name__)
logger.setLevel(DEBUG)
logger.addHandler(hand)

# pylint: disable-msg=W0613
@log_debug
def debug_me(arg, kwarg=2):
    pass
    
def test_log_debug():
    debugmestr = 'debug_me:  (1, 2) '
    debugmename = "tests.test_logging"
    # call function to generate log message
    debug_me(1, 2)
    # check the last generated log event
    assert debugmename == hand.event[0]
    assert debugmestr == hand.event[1]

