import abc


from askap.parset import ParameterSet, decode
from askap.opl.ptf.config import ParsetConfig

class Synthesizer(object, ParsetConfig):
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def set_sky_freq(self, freq):
        return

    @abc.abstractmethod
    def set_lo_freq(self, freq):
        return

    @abc.abstractmethod
    def set_lo_power(self, power):
        return

    @abc.abstractmethod
    def get_sky_freq(self):
        return

    @abc.abstractmethod
    def get_lo_freq(self):
        return

    @abc.abstractmethod
    def get_lo_power(self):
        return


class DummySynthesizer(Synthesizer):
    def __init__(self, lofreq=None, skyfreq=None, lopower=None, parset=None):
        if isinstance(parset, ParameterSet):
            self.initialize(parset)

        self._lo_freq = 0.0
        self._sky_freq = 0.0
        self._lo_power = 1.0

    def set_sky_freq(self, freq):
        self._sky_freq = freq

    def set_lo_freq(self, freq):
        self._lo_freq = freq

    def set_lo_power(self, power):
        self._lo_power = power

    def get_sky_freq(self):
        return self._sky_freq

    def get_lo_freq(self):
        return self._lo_freq

    def get_lo_power(self):
        return self._lo_power
