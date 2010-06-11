# Copyright (c) 2009 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
#
"""
========================================================
Module :mod:`askap.logging` -- Python logging extensions
========================================================

This module import all of python's built-in logging module and add the
following functionality:

    * :func:`log_debug` - a decorator for logging at the DEBUG level.

.. todo::

    python < 2.6.3 has a bug where::

        from logging import *

    doesn't import all symbols. We can remove the explicit imports once
    everyone is using version above 2.6.2

"""
import os
# pylint: disable-msg=W0401
from logging import *
# python 2.6 now has __all__, which means not everything gets in
# this is a bug fixed in 2.6.3
from logging import config
from logging import handlers
from logging import getLogger, basicConfig, getLevelName

# decorator
def log_debug(func):
    """Logging decorator for quick debugging of function calls and their arguments.
You need to set the logging level to ''DEBUG'' to see these message and
should have a logging instance called ''logger''.

Example::

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

def init_logging(args):
    """Parse `args` for --log-config=filename and initialise loggers using
    the configuration file.

        :param args: a list of arguments usually sys.argv

    """
    key = "--log-config"
    fname = "askap.log_cfg"
    for arg in args:
        if arg.startswith(key):
            k, v = arg.split("=")
            if os.path.exists(v):
                fname = v
            break

    if os.path.exists(fname):
        config.fileConfig(fname)
    else:
        raise IOError("No log configuration file found")
