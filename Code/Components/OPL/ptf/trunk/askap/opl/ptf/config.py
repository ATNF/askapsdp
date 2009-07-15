"""
ParameterSet file::

    # General set up
    ptf.project             =
    ptf.observer            =
    ptf.data_directory      =
    ptf.logger.type         =
    ptf.logger.level        =

    # CABB initial values
    ptf.cabb.weights_file   =
    ptf.cabb.attenuator     = [(port[i], value[i]), ...]
    ptf.cabb.test_signal    =

    # Synthesiser initial values
    ptf.synthesizer.sky_freq =
    ptf.synthesizer.lo_freq  =

    # Digitizer
    ptf.digitizer.test_signal =
    ptf.digitizer.delay       = [(port[i], value[i]), ...]
"""
from askap.opl.ptf import logger
from askap.parset import ParameterSet

class Config(object):
    def __init__(self, pfile):
        """
        Read the configuration from a ParameterSet file

        :param pfile: the ParameterSet file.

        """
        self._pset = ParameterSet(pfile).ptf

    def get_digitizer(self):
        if "digitizer" in self._pset:
            return self._pset.digitizer
        return ParameterSet()

    def get_cabb(self):
        if "cabb" in self._pset:
            return self._pset.cabb
        return ParameterSet()

    def get_synthesizer(self):
        if "synthesizer" in self._pset:
            return self._pset.synthesizer
        return ParameterSet()

    def get_logger(self):
        if "logger" in self._pset:
            return self._pset.logger
        return ParameterSet()

class ParsetConfig(object):
    """
    Any subsystem implementation needs to inherit from this class to support
    initialization from ParameterSet files. This requires a direct match
    for ParameterSet key to class method name e.g.::

        synthesizer.lo_freq -> Synthesizer.set_lo_freq

    """
    def initialize(self, parset):
        for k,v in parset.iteritems():
            getattr(self, "set_"+k)(decode(v))
