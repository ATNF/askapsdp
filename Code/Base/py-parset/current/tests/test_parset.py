# Copyright (c) 2009 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
#
import os
from askap.parset import ParameterSet, decode, encode
from nose.tools import raises, assert_equals

# default constructor test
def test_constructor():
    p = ParameterSet()


def test_stringconstructor():
    pset = """a.b.c = 1
a.b.d = x"""
    p = ParameterSet(pset)
    assert_equals(p.a.b.c, 1)
    
# from file
def test_fileconstructor():
    testfile = os.path.join(os.path.split(__file__)[0], 'example.parset')
    p = ParameterSet(testfile)

# from file
def test_dictconstructor():
    p1 = ParameterSet({'x.y': 1})
    p2 = ParameterSet(**{'x.y': 1})

# call constructor with args
def constructor(args):
    if not hasattr(args, "__len__"):
        args = (args,)
    p = ParameterSet(*args)

# generator for constructor arguments
def test_args():
    for t in [('x.y', 1), ('x', [1,2])]:
        yield constructor, t

@raises(OSError)
def test_failconstructor():
    p = ParameterSet("xxx.parset")

# test set_value
def test_set_value():
    p = ParameterSet()
    p.set_value("x.y", 1)
    p.set_value("x.z", 2)

# test __str__ function
def test_str():
    key0 = 'x.y'
    value0 = 1
    key1 = 'x.z'
    value1 = 2
    p = ParameterSet(key1, value1)
    p.set_value(key0, value0)
    assert_equals(str(p), "%s = %d\n%s = %d" % (key0, value0, key1, value1))

def test_to_dict():
    p = ParameterSet(x=1, y=2)
    d = p.to_dict()
    assert d == {'x': 1, 'y': 2}

def test_to_flat_dict():
    din = {'x.y':1, 'x.z':2}
    p = ParameterSet(**din)
    d = p.to_flat_dict()
    assert_equals(d.keys(), din.keys())


# test getting sub-parset "node"
def test_slice():
    p = ParameterSet('x.y.z', 1)
    assert isinstance(p.x.y, ParameterSet)

def test_get_value():
    p = ParameterSet('x.y.z', 1)
    v = p.get_value('x.y.z')
    v = p.get_value('x')
    v = p['x']
    v = p['x.y']
    v = p.x
    v = p.x.y.z
    # test default value for non existing key
    assert p.get_value('x.y.x', 10) == 10

def test_decoded():
    p = ParameterSet('x.y.z', '1')
    assert_equals(p.get_value('x.y.z'), 1)
    assert_equals(p['x.y.z'], 1)
    assert_equals(p.x.y.z, 1)

@raises(KeyError)
def test_get_fail():
    p = ParameterSet()
    v = p.get_value('x.y.x')
    v = p['x.y.x']

def test_in():
    p = ParameterSet('x.y.z', 1)
    assert 'x' in p
    assert 'x.y' in p
    assert 'x.y.z' in p
    assert 'a' not in p

# check decoded value
def decoder(k, v):
    assert_equals(decode(k), v)

# test generator for parset expressions
def test_decode():
    expr = {'1..9': range(1, 10),
            '[1..9]': range(1, 10),
            '9..1': range(9, 0, -1),
            'abc1..2.txt': ['abc1.txt', 'abc2.txt'],
            '[abc1..2.txt]': ['abc1.txt', 'abc2.txt'],
            '[1,2]': [1,2],
            '[x, y]': ['x', 'y'],
            '[1, x]': [1, 'x'],
            '[[1,2], [3,4]]': [[1,2], [3,4]],
            'true': True,
            'false': False,
            '[3 * false]': 3*[False],
            '1e10': 1e10,
            '.1': 0.1,
            '-1.0e-10': -1.0e-10,
            '[1, 2, "x y"]': [1,2, "x y" ]
            }

    for k,v in expr.items():
        yield decoder, k, v

# check encoded value
def encoder(k, v):
    assert_equals(encode(k), v)

# test generator for parset expressions
def test_encode():
    expr = {'1..9': range(1, 10),
            '[1, 2]': [1,2],
            '[x, y]': ['x', 'y'],
            '[1, x]': [1, 'x'],
            '[[1, 2], [3, 4]]': [[1,2], [3,4]],
            '[3 * false]': 3*[False],
            'true': True,
            'false': False,
            '-1e-10': -1.0e-10,
            '[1, 2, "x y"]': [1,2, "x y" ]
          
            }
    for k,v in expr.items():
        yield encoder, v, k

def test_keys():
    keys = ['a', 'x.y.z']
    p = ParameterSet(keys[1], 1)
    p.set_value(keys[0], 1)
    assert_equals(p.keys(), keys)

def test_items():
    keys = ['x.a', 'x.y.z']
    p = ParameterSet('x.y.z', 1)
    p.set_value('x.a', 2)
    assert_equals(p.items(), [('x.a', 2), ('x.y.z', 1)])
