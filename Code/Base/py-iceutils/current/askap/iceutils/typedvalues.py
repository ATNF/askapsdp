# Copyright (c) 2013 CSIRO
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
__all__ = ["TypedValueMapper", "TimeTaggedTypedValueMapper",
           "typed_mapper", "dict_mapper"]

import Ice
import types

# pylint: disable-msg=W0611
from askap.slice import TypedValues

# ice doesn't agree with pylint
# pylint: disable-msg=E0611
import askap.interfaces as iceint
        

def dict_mapper(data):
    """Mapper from `TypeValueMap` to :class`dict`"""
    out = {}
    for k, v in data.items():
        if v.type in (iceint.TypedValueType.TypeDoubleComplex,
                      iceint.TypedValueType.TypeFloatComplex):
            out[k] = complex(v.value.real, v.value.imag)
        elif v.type in (iceint.TypedValueType.TypeDoubleComplexSeq,
                      iceint.TypedValueType.TypeFloatComplexSeq):
            out[k] = [ complex(i.real, i.imag) for i in v.value ]
        elif v.type == iceint.TypedValueType.TypeDirection:
            out[k] = (v.value.coord1, v.value.coord2, str(v.value.sys))
        elif v.type == iceint.TypedValueType.TypeNull:
            out[k] = None
        else:
            out[k] = v.value
    return out


class TypedValueMapper(object):
    """Mapper class from :class:`dict` to `TypeValueMap`.

    Example::
    
            mapper = TypedValueMapper()
            mapper.register_double_complex(["test"])
            d = {'on_source': True,
            'scan_id' : -1, 
            'pointing_radec': (0.0, -1.0, 'J2000'),
            'test_value': ["S", "T"],
            'test': 1+1j,
            'test2': 1.0,
            }
            print mapper(d)

    """
    def __init__(self):
        self._mapping = {}
        
    def _auto_type(self, v):
        typ = type(v)
        typename = typ.__name__.capitalize()
        # inspect elements to determine type
        if typ in (types.ListType, types.TupleType):
            # special case for Direction type
            if (len(v) == 3 and type(v[0]) is types.FloatType
                and type(v[1]) is types.FloatType
                and type(v[2]) is types.StringType):
                return self._as_direction(v)
            # Lists are "Seq" in ASKAP Ice
            typ = type(v[0])
            # String is str in python
            if typ is types.StringType:
                typename = "String"
            else:
                if typ is types.ComplexType:
                    typename = "FloatComplex"
                    v = [iceint.FloatComplex(i.real, i.imag) for i in v]
                else:
                    typename = typ.__name__.capitalize()
            typename += "Seq"

        elif typ is types.ComplexType:
            # create complex
            v = iceint.FloatComplex(v.real, v.imag)
            return self._to_typed_value("FloatComplex", v)
        elif typ is types.StringType:
            typename = "String"
        elif typ is types.NoneType:
            return iceint.TypedValue()
#        else:
#            raise TypeError("Unsupported type {0}.".format(typ))
        return self._to_typed_value(typename, v) 

    def register_double(self, keys):
        """Use python float value for given keys and turn it into a
        TypeDouble as the default is float."""
        if not isinstance(keys, list):
            keys = [keys]
        for k in keys:
            self._mapping[k] = self._as_double

    def register_double_complex(self, keys):
        """For the given `keys` use python complex value and turn it into a
        TypeDoubleComplex as the default is FloatComplex"""
        if not isinstance(keys, list):
            keys = [keys]
        for k in keys:
            self._mapping[k] = self._as_double_complex

    def _as_double(self, v):
        # float and double are the same in python
        return self._to_typed_value("Double", v)

    def _as_double_complex(self, v):
        # float and double are the same in python
        tname = "DoubleComplex"
        if type(v) in (types.ListType, types.TupleType):
             v = [iceint.FloatComplex(i.real, i.imag) for i in v]
             tname += "Seq"
        else:
            v = iceint.DoubleComplex(v.real, v.imag)
        return self._to_typed_value(tname, v)

    def _to_typed_value(self, name, value):
        cls = getattr(iceint, "TypedValue"+name)
        typ = getattr(iceint.TypedValueType, "Type"+name)
        return cls(typ, value)

    def _as_direction(self, value):
        dv = iceint.Direction(value[0], value[1], getattr(iceint.CoordSys,
                                                         value[2]))
        return self._to_typed_value("Direction", dv)

    def __call__(self, data):
        """Turn a dict into a TypedValueMap"""
        tvmap = {}
        for k, v in data.items():
            if k in self._mapping:
                tvmap[k] = self._mapping[k](v)
            else:
                tvmap[k] = self._auto_type(v)
        return tvmap



class TimeTaggedTypedValueMapper(TypedValueMapper):
    def __call__(self, bat, data):
        return iceint.TimeTaggedTypedValueMap(bat, 
                                              TypedValueMapper.__call__(self,
                                                                        data))

def typed_mapper(d, bat=None):
    if bat is not None:
        return TimeTaggedTypedValueMapper()(bat, d)
    return TypedValueMapper()(d)

    
if __name__ == "__main__":

    d = {'on_source': True,
         'scan_id' : -1, 
         'pointing_radec': (0.0, -1.0, 'J2000'),
         'test_value': ["S", "T"],
         'test': 1+1j,
         'test2': 1.0,
         'test3': None,
         }
    tvm = typed_mapper(d)
    print dict_mapper(tvm)
    print d
