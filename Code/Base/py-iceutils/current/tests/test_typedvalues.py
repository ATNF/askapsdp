#!/usr/bin/env python
import sys
import os

from nose.tools import assert_equals, assert_items_equal
# pylint: disable-msg=E0611
from askap.iceutils import IceSession

# pylint: disable-msg=W0611
from askap.slice import TypedValues

# ice doesn't agree with pylint
# pylint: disable-msg=E0611
from  askap.interfaces import TypedValueType as TVT
import askap.interfaces as AI

from askap.iceutils.typedvalues import *
from askap.interfaces import Direction, CoordSys

def tmap(tvitem, dictitem):
    
    assert_equals(dict_mapper(tvitem), dictitem)
    

def test_dict_mapper():
    vals = ( 
        (AI.TypedValue(), None),
        (AI.TypedValueFloat(TVT.TypeFloat, 1.0), 1.0),
        (AI.TypedValueFloatSeq(TVT.TypeFloatSeq, [1.0]) , [1.0]),
        (AI.TypedValueDouble(TVT.TypeDouble, 1.0) , 1.0),
        (AI.TypedValueDoubleSeq(TVT.TypeDoubleSeq, [1.0]) , [1.0]),
        (AI.TypedValueInt(TVT.TypeInt, 1) , 1),
        (AI.TypedValueLong(TVT.TypeLong, 1l) , 1),
        (AI.TypedValueString(TVT.TypeString, 'y') , 'y'),
        (AI.TypedValueBool(TVT.TypeBool, True) , True),
        (AI.TypedValueDirection(TVT.TypeDirection, 
                                Direction(0.0, 0.0, CoordSys.J2000)),
         (0.0, 0.0, 'J2000')),
        )

    for i, j in vals:
        yield tmap, {'x':i}, {'x':j}

def test_typedvaluemapper():
    mapper = TypedValueMapper()
    mapper.register_double_complex(["double_complex"])
    mapper.register_double(("double", "doubleseq"))
    d = {
        'bool': True, 'boolseq': [True]*2,
        'int' : -1, 'intseq' : [-1]*2, 
        'direction1': (0.0, -1.0, 'J2000'),
        'direction2': (0.0, -1.0, 'AZEL'),
        'string': 'test', 'stringseq': ["S", "T"],
        'double_complex': 1+1j,
        'complex': 1+1j,
        'float': 1.0, 'floatseq': [1.0]*2, 
        'double': 1.0, 'doubleseq': [1.0]*2, 
        'long': 1l,
        'none': None,
         }
    mapped = mapper(d)
    assert_items_equal(mapped, dict_mapper(mapped))

