from askap import logging
from askap.parset import ParameterSet
from askap.opl.ptf import logger as ptflogger
from askap.opl.ptf.subsystems import *

class Control(object):
    def __init__(self, parsetfile=None):
        if parsetfile is not None:
            self._cfg = ParameterSet(parsetfile)
        else:
            self._cfg = ParameterSet()

        self._logger = self._init_subsystem("logger", "StdoutLogger")
        self.synthesizer = self._init_subsystem("synthesizer")
        self.cabb = self._init_subsystem("cabb")
        self.digitizer = None #self._init_subsystem("digitizer")
        self.antenna = None
        self.datarecorder = None
        ptflogger.info("Initialized system")

    def _init_subsystem(self, name, default=None):
        import inspect
        if default is None:
            default = "Sim"+name.capitalize()
        stype = self._cfg.get_value(".".join(["ptf", name, "type"]), default)
        cls = inspect.currentframe().f_globals.get(stype)
        return cls(parset=self._cfg)

    def set_synthesizer(self, obj):
        self.synthesizer = obj

    def set_digitizer(self, obj):
        self.digitizer = obj

    def set_logger(self, obj):
        self.logger = obj

    def set_cabb(self, obj):
        self.cabb = obj

    def set_datarecorder(self, obj):
        self.datarecorder = obj

    def set_antenna(self, obj):
        self.antenna = obj
