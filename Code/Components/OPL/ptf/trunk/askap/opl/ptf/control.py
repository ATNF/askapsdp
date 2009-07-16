from askap import logging
from askap.opl.ptf import logger
from askap.opl.ptf.subsystems import *
from askap.opl.ptf.config import Config


class Control(object):
    def __init__(self, project=None, user=None, parsetfile=None):
        self._cfg = Config(parsetfile)
        self._logger = self._init_subsystem("logger", "StdoutLogger")
        self._synthesizer = self._init_subsystem("synthesizer")
        self._digitizer = self._init_subsystem("digitizer")
        self._cabb = self._init_subsystem("cabb")
        self._antenna = None
        self._datarecorder = None

    def _init_subsystem(self, name, default=None):
        if default is None:
            default = "Sim"+name.capitalize()
        stype = self._cfg.get_value(name+".type", default)
        cls = eval(stype)
        return cls(parset=self._cfg.get_value(name))

    def set_synthesizer(self, obj):
        self._synthesizer = obj

    def set_digitizer(self, obj):
        self._digitizer = obj

    def set_logger(self, obj):
        self._logger = obj

    def set_cabb(self, obj):
        self._cabb = obj

    def set_datarecorder(self, obj):
        self._datarecorder = obj

    def set_antenna(self, obj):
        self._antenna = obj

    def set_project(self, name):
        self._project = name

    def get_project(self):
        return self._project

    def set_user(self, name=None):
        if name is None:
            self._user = os.environ.get("USER", "unknown")
        else:
            self._user = name
