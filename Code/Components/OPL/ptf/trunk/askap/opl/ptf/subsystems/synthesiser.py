__all__ = ["SimSynthesiser"]

import abc

from askap import logging
from askap.opl.ptf.config import init_from_pset

logger = logging.getLogger(__name__)

class Synthesiser(object):
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

    lo_power = abc.abstractproperty(get_lo_power, set_lo_power)
    lo_freq = abc.abstractproperty(get_lo_freq, set_sky_freq)
    sky_freq = abc.abstractproperty(get_sky_freq, set_sky_freq)

@init_from_pset
class SimSynthesiser(Synthesiser):
    def __init__(self, lofreq=None, skyfreq=None, lopower=None, parset=None):
        # default values
        self._lo_freq = 0.0
        self._sky_freq = 0.0
        self._lo_power = 1.0
        # parameter set defaults
        self.init_from_pset(parset, prefix="ptf.synthesiser")
        # overwrites
        if lofreq is not None:
            self._lo_freq = lofreq
        if skyfreq is not None:
            self._sky_freq = skyfreq
        if lopower is not None:
            self._lo_power = lopower
        logger.info("Initialized")

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

    lo_power = property(get_lo_power, set_lo_power)
    lo_freq = property(get_lo_freq, set_sky_freq)
    sky_freq = property(get_sky_freq, set_sky_freq)
