__all__ = ["SimDigitiser"]

import abc

from askap import logging
from askap.opl.ptf.config import init_from_pset

logger = logging.getLogger(__name__)

class Digitiser(object):
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def get_adc_samples(self, port):
        return

    @abc.abstractmethod
    def get_pfb_samples(self, port):
        return

    @abc.abstractmethod
    def get_delay(self, ports):
        return

    @abc.abstractmethod
    def set_delay(self, portvaluepairs):
        return

    @abc.abstractmethod
    def get_test_signal(self, ports):
        return

    @abc.abstractmethod
    def set_test_signal(self, portvaluepairs):
        return


@init_from_pset
class SimDigitiser(Digitiser):
    def __init__(self, parset=None):
        # default values
        self._adc = [0.0]*48
        self._delay = [0]*48
        self._test = [False]*48
        # parameter set defaults
        self.init_from_pset(parset, prefix="ptf.digitiser")
        # overwrites

        logger.info("Initialized")

    def get_adc_samples(self, port):
        return [0]*8192

    def get_pfb_samples(self, port):
        return [0+0j]*8192

    def get_delay(self, ports):
        if ports is None:
            return self._delay[:]
        if isinstance(ports, int):
            ports = [ports]
        return [self._delay[i] for i in ports]

    def set_delay(self, portvaluepairs):
        if not isinstance(portvaluepairs[0], list):
            portvaluepairs = [portvaluepairs]
        for pair in portvaluepairs:
            if 0 <= pair[0] < len(self._delay):
                self._delay[pair[0]] = pair[1]

    def get_test_signal(self, port):
        if ports is None:
            return self._test[:]
        if isinstance(ports, int):
            ports = [ports]
        return [self._test[i] for i in ports]

    def set_test_signal(self, portvaluepair):
        if not isinstance(portvaluepairs[0], list):
            portvaluepairs = [portvaluepairs]
        for pair in portvaluepairs:
            if 0 <= pair[0] < len(self._test):
                self._test[pair[0]] = pair[1]
