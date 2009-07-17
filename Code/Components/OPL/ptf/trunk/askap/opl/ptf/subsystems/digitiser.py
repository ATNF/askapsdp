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
    def get_delay(self, port):
        return

    @abc.abstractmethod
    def set_delay(self, portdelaypair):
        return

    @abc.abstractmethod
    def get_test_signal(self, port):
        return

    @abc.abstractmethod
    def set_test_signal(self, portvaluepair):
        return


@init_from_pset
class SimDigitiser(Digitiser):
    def __init__(self, parset=None):
        # default values

        # parameter set defaults
        self.init_from_pset(parset, prefix="ptf.Digitiser")
        # overwrites

        logger.info("Initialized")
