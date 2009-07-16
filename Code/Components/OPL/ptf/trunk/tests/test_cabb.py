#!/usr/bin/env python
from askap.opl.ptf.subsystems import SimCabb
from askap.parset import ParameterSet

def create_parset():
    p = ParameterSet()
    p.set_value('ptf.cabb.attenuator', [[0, 31]])
    p.set_value('ptf.cabb.test_signal', [0, True])
    return p

def test_init():
    s = SimCabb()
    assert s.get_attenuator([0]) == [[0, 0]]
    assert s.get_test_signal(0) == False


def test_parset_init_with_overwrite():
    s = SimCabb(parset=create_parset())
    assert s.get_attenuator([0]) == [[0,31]]
    assert s.get_test_signal(0) == True
