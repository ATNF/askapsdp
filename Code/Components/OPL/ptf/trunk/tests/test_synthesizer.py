#!/usr/bin/env python
from askap.opl.ptf.subsystems.synthesizer import SimSynthesizer
from askap.parset import ParameterSet

def create_parset():
    p = ParameterSet()
    p.set_value('lo_freq', 11.0)
    p.set_value('sky_freq', 22.0)
    p.set_value('types', "Sim")

    return p.to_dict()

def test_init():
    s = SimSynthesizer()
    assert s.get_lo_freq() == 0.0
    assert s.get_sky_freq() == 0.0
    assert s.get_lo_power() == 1.0

def test_parset_init_with_overwrite():
    s = SimSynthesizer(lopower=33.0, pdict=create_parset())
    assert s.get_lo_freq() == 11.0
    assert s.get_sky_freq() == 22.0
    assert s.get_lo_power() == 33.0
