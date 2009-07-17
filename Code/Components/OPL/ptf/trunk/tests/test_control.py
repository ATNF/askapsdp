from askap.opl.ptf.control import Control
from nose.tools import assert_equals

def test_init():
    c = Control()

def test_init_parset():
    c = Control(parsetfile="tests/control.parset")
    assert_equals(c.cabb.get_attenuator(0), [16])
