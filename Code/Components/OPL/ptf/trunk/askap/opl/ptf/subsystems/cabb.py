__all__ = ["SimCabb"]

import abc

from askap import logging
from askap.opl.ptf.config import init_from_pset

logger = logging.getLogger(__name__)

class Cabb(object):
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def set_attenuator(self, portvaluepairs):
        return

    @abc.abstractmethod
    def get_attenuator(self, ports):
        return

    @abc.abstractmethod
    def set_test_signal(self, portvaluepair):
        return

    @abc.abstractmethod
    def get_test_signal(self, port):
        return

    @abc.abstractmethod
    def set_beamformer_weights(self, filename):
        return

    @abc.abstractmethod
    def get_beamformer_weights(self, mode):
        return

    @abc.abstractmethod
    def start_correlation_capture(self):
        return

    @abc.abstractmethod
    def stop_correlation_capture(self):
        return

    @abc.abstractmethod
    def start_acm_capture(self, filename, nacc, **kw):
        return

    @abc.abstractmethod
    def stop_acm_capture(self):
        return

@init_from_pset
class SimCabb(Cabb):
    def __init__(self, parset=None):
        self._attn = [8]*48
        self._test = [False]*48
        self.init_from_pset(parset, prefix="ptf.cabb")

    def set_attenuator(self, portvaluepairs):
        if not isinstance(portvaluepairs[0], list):
            portvaluepairs = [portvaluepairs]
        for pair in portvaluepairs:
            if 0 <= pair[0] < len(self._attn):
                self._attn[pair[0]] = pair[1]

    def get_attenuator(self, ports=None):
        if ports is None:
            return self._attn[:]
        if isinstance(ports, int):
            ports = [ports]
        return [self._attn[i] for i in ports]

    def set_test_signal(self, portvaluepairs):
        if not isinstance(portvaluepairs[0], list):
            portvaluepairs = [portvaluepairs]
        for pair in portvaluepairs:
            if 0 <= pair[0] < len(self._test):
                self._test[pair[0]] = pair[1]

    def get_test_signal(self, ports=None):
        if ports is None:
            return self._test[:]
        if isinstance(ports, int):
            ports = [ports]
        return [self._test[i] for i in ports]

    def set_beamformer_weights(self, filename):
        if not isinstance(filename, str):
            #turn me into file
            pass
        # send file to CABB

    def get_beamformer_weights(self, mode='file'):
        modes = ["file", "matrix"]
        if mode.lower() not in modes:
            raise TypeError("'mode' must be one of %s" % modes)
        if mode == 'file':
            filename = "notimplemented.txt"
            return filename
        else:
            matrix = None
            return matrix

    def start_correlation_capture(self, filename=None, nacc=195, **kw):
        if filename is None:
            filename = "ptf_cor_yyyymmdd-hhmm-sourcename-skyfreqmhz-?.out"
        logger.warn("start_correlation_capture not implemented")

    def stop_correlation_capture(self):
        logger.warn("stop_correlation_capture not implemented")

    def start_acm_capture(self, filename):
        logger.warn("start_acm_capture not implemented")

    def stop_acm_capture(self):
        logger.warn("stop_acm_capture not implemented")
