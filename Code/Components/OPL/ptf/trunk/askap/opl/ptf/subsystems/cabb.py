__all__ = ["SimCabb"]

import abc

from askap import logging
from askap.opl.ptf.config import init_from_pset

logger = logging.getLogger(__name__)

class Cabb(object):
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def set_attenuator(self, portvalpairs):
        return

    @abc.abstractmethod
    def get_attenuator(self, ports):
        return

    @abc.abstractmethod
    def set_test_signal(self, port, value):
        return

    @abc.abstractmethod
    def get_test_signal(self, port):
        return

    @abc.abstractmethod
    def write_beamformer_weights(self, filename):
        return

    @abc.abstractmethod
    def get_beamformer_weights(self):
        return

    @abc.abstractmethod
    def start_correlation_capture(self):
        return

    @abc.abstractmethod
    def stop_correlation_capture(self):
        return

    @abc.abstractmethod
    def start_acm_capture(self, filename):
        return

    @abc.abstractmethod
    def stop_acm_capture(self):
        return

@init_from_pset
class SimCabb(Cabb):
    def __init__(self, parset=None):
        self._attn = [0]*48
        self._test = [False]*48
        self.init_from_pset(parset, prefix="ptf.cabb")

    def set_attenuator(self, portvalpairs):
        for pair in portvalpairs:
            if 0 <= pair[0] < len(self._attn):
                self._attn[pair[0]] = pair[1]

    def get_attenuator(self, ports):
        out = []
        for i in ports:
            out.append([i, self._attn[i]])
        return out

    def set_test_signal(self, portvalpair):
        self._test[portvalpair[0]] = portvalpair[1]

    def get_test_signal(self, port):
        return self._test[port]

    def write_beamformer_weights(self, filename):
        pass

    def get_beamformer_weights(self):
        pass

    def start_correlation_capture(self):
        pass

    def stop_correlation_capture(self):
        pass

    def start_acm_capture(self, filename):
        pass

    def stop_acm_capture(self):
        pass
