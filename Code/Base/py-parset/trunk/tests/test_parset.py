import os
from askap.parset import ParameterSet, decode
from nose.tools import raises

# default constructor test
def test_constructor():
    p = ParameterSet()

# from file
def test_fileconstructor():
    testfile = os.path.join(os.path.split(__file__)[0], 'example.parset')
    p = ParameterSet(testfile)

# call constructor with args
def constructor(args):
    if not hasattr(args, "__len__"):
        args = (args,)
    p = ParameterSet(*args)

# generator for contructor arguments
def test_args():
    for t in [('x.y', 1), ('x', [1,2])]:
        yield constructor, t

# test __str__ function
def test_str():
    key = 'x.y'
    p = ParameterSet(key, 1)
    assert(p.x.y == 1)

# test getting sub-parset "node"
def test_slice():
    p = ParameterSet('x.y.z', 1)
    assert isinstance(p.x.y, ParameterSet)

# check decoded value
def decoder(k, v):
    assert decode(k) == v

# test generator for parset expressions
def test_decode():
    expr = {'1..9': range(1, 10),
            '9..1': range(9, 0, -1),
            'abc1..2.txt': ['abc1.txt', 'abc2.txt'],
            '[1,2]': [1,2],
            '[x, y]': ['x', 'y'],
            '[1, x]': [1, 'x'],
            '[[1,2], [3,4]]': [[1,2], [3,4]],
            'true': True,
            'false': False,
            '1e10': 1e10,
            '.1': 0.1,
            '-1.0e-10': -1.0e-10
            }

    for k,v in expr.items():
        yield decoder, k, v
