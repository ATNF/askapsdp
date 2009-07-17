#!/usr/bin/env python
from askap.opl.ptf.subsystems import SimSynthesiser
from askap.parset import ParameterSet

def create_parset():
    p = ParameterSet()
    p.set_value('ptf.synthesiser.lo_freq', 11.0)
    p.set_value('ptf.synthesiser.sky_freq', 22.0)
    return p

def test_init():
    s = SimSynthesiser()
    assert s.get_lo_freq() == 0.0
    assert s.get_sky_freq() == 0.0
    assert s.get_lo_power() == 1.0

def test_parset_init_with_overwrite():
    s = SimSynthesiser(lopower=33.0, parset=create_parset())
    assert s.lo_freq == 11.0
    assert s.sky_freq == 22.0
    assert s.lo_power == 33.0
