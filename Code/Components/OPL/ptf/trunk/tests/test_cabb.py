#!/usr/bin/env python
from askap.opl.ptf.subsystems import SimCabb
from askap.parset import ParameterSet

from nose.tools import assert_equals

def create_parset():
    p = ParameterSet()
    p.set_value('ptf.cabb.attenuator', [[0, 31]])
    p.set_value('ptf.cabb.test_signal', [[0, True]])
    return p

def test_init():
    s = SimCabb()
    assert_equals(s.get_attenuator([0]), [8])
    assert_equals(s.get_attenuator(1), [8])
    assert_equals(s.get_attenuator([0,1]), [8]*2)
    assert_equals(s.get_test_signal(0), [False])
    assert_equals(s.get_attenuator(), [8]*48)


def test_parset_init_with_overwrite():
    s = SimCabb(parset=create_parset())
    assert_equals(s.get_attenuator([0]), [31])
    assert_equals(s.get_test_signal(0), [True])
