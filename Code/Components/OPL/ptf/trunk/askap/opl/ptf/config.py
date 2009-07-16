"""
ParameterSet file::

    ptf.logger.type         = <file/console>
    ptf.logger.level        = INFO
    ptf.logger.format.msg   = (asctime)s %(levelname)s %(name)s - %(message)s
    ptf.logger.format.date  =

    # datarec
    ptf.datarec.type        = <SimDataRecorder>
    ptf.datarec.project     =
    ptf.datarec.basedir     =
    ptf.datarec.use_date    = true

    # CABB initial values
    ptf.cabb.type           = <SocketCABB/SimCABB>
    ptf.cabb.beamformer_weights   =
    ptf.cabb.attenuator     = <[[port[i], value[i]], ...]>
    ptf.cabb.test_signal    = <[port, value]>

    # Synthesiser initial values
    ptf.synthesizer.type     = <SimSynthesizer/EpicsSynthesizer>
    ptf.synthesizer.sky_freq = <integer>
    ptf.synthesizer.lo_freq  = <integer>
    ptf.synthesizer.lo_power = <float>

    # Digitizer
    ptf.digitizer.type        = <SimDigitizer>
    ptf.digitizer.test_signal =
    ptf.digitizer.delay       = <[(port[i], value[i]), ...]>

"""
from askap.opl.ptf import logger
from askap.parset import ParameterSet

class Config(object):
    def __init__(self, pfile):
        """
        Read the configuration from a ParameterSet file

        :param pfile: the ParameterSet file.

        """
        pset = ParameterSet(pfile).ptf
        for subsystem in ["digitizer", "synthesizer", "datarec", "cabb",
                          "logger", "common"]:
            v = None
            if subsystem in pdict:
                v = pset[subsystem]
            setattr(self, subsystem, v )

        logger.info("Read Parameter Set file '%s'" % pfile)


def init_from_pset(cls):
    """
    Decorator
    """
    def initialize(cls, pset, prefix=""):
        """
        Any subsystem implementation needs to inherit from this class to support
        initialization from ParameterSet dicts. This requires a direct match
        for dictionary key to class method name e.g.::

            synthesizer.lo_freq = value -> Synthesizer.set_lo_freq(value)

        or for multiple arguments::

            aaa.bbb.xxx = value1
            aaa.bbb.yyy = value2   -> Aaa.set_bbb(xxx=value1, yyy=value2)


        """
        if pset is None:
            return
        if prefix:
            if prefix not in pset:
                return
            pset = pset[prefix]
        pdict = pset.to_dict()
        for (k,v) in pdict.items():
            skey = "set_"+k
            # filter non-valid entries and _special_ key 'type'
            if k == "type" or not hasattr(cls, skey):
                continue
            method = getattr(cls, skey)
            if isinstance(v, dict):
                method(**v)
            else:
                method(v)

    setattr(cls, "init_from_pset", initialize)
    return cls
