from askap.opl.ptf.control import Control


def test_init():
    c = Control()

def test_init_parset():
    c = Control(parsetfile="tests/control.parset")
    assert c.cabb.get_attenuator([0]) == [[0,16]]
