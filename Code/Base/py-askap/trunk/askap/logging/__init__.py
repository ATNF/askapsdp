## @file
#  Wrapper around the standard logging package and the XMLLayout module for 
#  adding a XMLSocketAppender sending log4j xml formatted eevents
#
# copyright (c) 2007 ASKAP. All Rights Reserved.
# @author Malte Marquarding <malte.marquarding@csiro.au>
#

# make this module look like logging
# since we know what we are doing allow wildcard
# pylint: disable-msg=W0401
from logging import *

# insert xml socket handler
from xmllayout import RawSocketHandler as XMLSocketHandler

# decorator
def log_debug(func):
    """Logging decrorator for quick debugging of function calls and their arguments.
You need to set the logging level to ''DEBUG'' to see these message ans should have a logging instance called ''logger''.

Example: ..

from askap.logging import getLogger, log_debug

# use module name as logger name
logger = getLogger(__name__)

class Foo:
    def __init__(self):
        pass

    @log_debug
    def multiply(self, x, y=2):
        return x*y
    
"""
    import inspect
    # retrieve logger from calling scope
    logger = inspect.currentframe().f_back.f_globals.get("logger", None)
    if not logger:
        logger = getLogger("Unspecified")
        basicConfig(level=DEBUG)
    fname = func.func_name
    def postlog(*args, **kwargs):
        logname = ""
        outargs = None
        # check if we are a method of a class
        if len(args) > 0 and args[0].__class__.__dict__.has_key(fname):
            logname = args[0].__class__.__name__+"."
            # remove self from args output
            outargs = args[1:]
        else:
            outargs = args
        logname += fname
        logger.log(DEBUG, 
                   "%s:  %s %s" % (logname, str(outargs or ""), str(kwargs or ""))) 
        # pylint: disable-msg=W0142
        return func(*args, **kwargs)
    return postlog
