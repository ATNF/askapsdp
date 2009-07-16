__all__ = ["FileLogger", "StdoutLogger"]

import sys
import abc
from askap import logging
from askap.opl.ptf import logger
from askap.opl.ptf.config import init_from_pset


class Logger(object):
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def set_format(self, msg, date):
        return

    @abc.abstractmethod
    def set_level(self, lvl):
        return

class FileLogger(object):
    @init_from_pset
    def __init__(parset=None):
        self._level = logging.INFO
        self._logger = logger
        self._fname = "/tmp/ptf-opl.log"
        self._fmt = logging.Formatter()
        self_.init_from_pset(parset)
        self._handler = logging.FileHandler(self._fname)
        self._logger.addHandler(self._handler)
        self._logger.setLevel(self._level)

    def set_format(self, msg, date):
        self._fmt = logging.Formatter(msg, fmt)

    def set_level(self, lvl):
        self._level = eval("logging.%s" % lvl.upper())

    def set_file_name(self, name):
        self._fname = name

class StdoutLogger(object):
    @init_from_pset
    def __init__(parset=None):
        self._level = logging.INFO
        self._logger = logger
        self._fmt = logging.Formatter()
        self_.init_from_pset(parset)
        self._handler = logging.StreamHandler(sys.stdout)
        self._logger.addHandler(self._handler)
        self._logger.setLevel(self._level)

    def set_format(self, msg, date):
        self._fmt = logging.Formatter(msg, fmt)

    def set_level(self, lvl):
        self._level = eval("logging.%s" % lvl.upper())
