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
import re

from askap.parset import logger

class ParameterSet(object):
    """
    The default constructor creates an empty ParameterSet instance.
    ParameterSet keys can be accessed as attributes::

        >>> p = ParameterSet('x.y.z', 1)
        >>> print p.x.y.z
        1

    or dictionary keys::

        >>> p = ParameterSet('x.y.z', 1)
        >>> print p["x"]["y"]["z"]
        1
        >>> print p["x.y.z"]
        1

    or using :meth:`get_value` which can be used with a default value if the
    key doesn't exists::

        >>> p = ParameterSet('x.y.z', 1)
        >>> print p.get_value('x.y.z')
        1
        >>> print p.get_value('x.y.a', 10)
        10

    To test if a key exists us the `in` keyword. Note that the key must start at
    the root value::

        >>> p = ParameterSet('x.y.z', 1)
        >>> print 'x' in p
        True
        >>> print 'y' in p
        False

    :param args: if only one argument, this is assumed to be the file name of
                 a ParameterSet file. If two arguments are provided this is
                 assumed to be a key and a value.
    :param kw:   key/value parameters

    Example::

        >>> p0 = ParameterSet()
        >>> p1 = ParameterSet('x.y', 1)
        >>> p2 = ParameterSet(x=1, y=2, z=ParameterSet('a', 3))
        >>> print x.a
        3
        >>> print x['a']
        3
        >>> print x.get_value('a')
        3
        >>> p3 = ParameterSet('xyz.parset')
        >>> p1.set_value('x.a', 2)

    """

    _reserved = ["get_value", "set_value", "to_dict", "to_flat_dict",
                 "keys", "items"]

    def __init__(self, *args, **kw):
        object.__setattr__(self, "_keys", [])
        # from file
        if len(args) == 1:
            if isinstance(args[0], basestring) and os.path.exists(args[0]):
                pfile = file(args[0], "r")
                i = 1
                for line in pfile:
                    pair = extract(line)
                    if pair:
                        try:
                            self.set_value(*pair)
                        except ValueError, ex:
                            raise ValueError("In line %d of %s. %s" % (i,
                                                                       args[0],
                                                                       ex.message))
                    i += 1
                logger.info("Read ParameterSet file %s" % args[0])
            else:
                raise ValueError("Given (single) argument is not a file name")
        # from key, value
        elif len(args) == 2:
            self.set_value(*args)
        elif len(kw):
            for k,v in kw.iteritems():
                self.set_value(k, v)
        elif len(args) == 0 and len(kw) == 0:
            pass
        else:
            raise ValueError("Incorrect arguments to constructor.")

    def get_value(self, k, default=None):
        """Return the value from the ParameterSet using an optional default
        value if the key doesn't exist.

        :param key: the key to get the value for
        :param default: the default value to return if the key doesn't exist

        """
        inkey = k
        k, tail = self._split(inkey)
        if k in self._keys:
            child = self.__dict__[k]
            if isinstance(child, self.__class__):
                if tail is not None:
                    return child.get_value(tail, default)
                else:
                    return decode(child)
            else:
                return decode(child)
        else:
            if default is None:
                raise KeyError("Key '%s' not found." % inkey )
            else:
                return default

    def keys(self):
        out = []
        for k in self._keys:
            child = self.__dict__[k]
            if isinstance(child, self.__class__):
                for key in child.keys():
                    out.append(".".join([k, key]))
            else:
                out.append(k)
        out.sort()
        return out


    def set_value(self, k, v):
        """
        Add or replace key/value pair. This will recursively create keys if
        necessary when the key contains '.' notation. This is the only way to
        add keys of this form. To set non-nested attributes one can use
        attribute or item set notation, so that the following are equivalent::

            p = ParameterSet()
            p.x = 1
            p["x"] = 1
            p.set_value("x", 1)
            # to add nested keys use
            p.set_value('x.y', 1)
            # this fails
            p.x.y = 1
            # as does this
            p["x"]["y"]

        :param k: key
        :param v: value

        """
        inkey = k
        k, tail = self._split(k)
        if k in self._reserved:
            raise KeyError("Key '%s' is a reserved keyword" % k)
        if k in self._keys:
            child = self.__dict__[k]
            if isinstance(child, self.__class__):
                child.set_value(tail, v)
            else:
                if tail:
                    raise ValueError("Leaf node %s can't be extended" % k)
                else:
                    self.__dict__[k] = v
        else:
            if not tail:
                self.__dict__[k] = v
            else:
                child = ParameterSet()
                child.set_value(tail, v)
                self.__dict__[k] = child
            self._keys.append(k)

    def __setitem__(self, k, v):
        self.set_value(k, v)

    def __setattr__(self, k, v):
        self.set_value(k, v)

    def __getitem__(self, k):
        return self.get_value(k)

    def _split(self, k):
        keys = k.split(".")
        k = keys[0]
        tail = None
        if len(keys) > 1:
            tail = ".".join(keys[1:])
        return k, tail

    def __contains__(self, k):
        k, tail = self._split(k)
        if k in self._keys:
            child = self.__dict__[k]
            if isinstance(child, self.__class__):
                if tail is not None:
                    return child.__contains__(tail)
                else:
                    return True
            else:
                return True
        else:
            return False


    def to_dict(self):
        """
        Returns a python :class:`dict` representation of the `ParameterSet`,
        decoding all values using :func:`decode`
        """
        out = {}
        for k in self._keys:
            if isinstance(self.__dict__[k], self.__class__):
                out[k] = self.__dict__[k].to_dict()
            else:
                out[k] = decode(self.__dict__[k])
        return out

    def to_flat_dict(self):
        """
        Returns a python :class:`dict` representation of the `ParameterSet`,
        with flat keys and encoded (string) values.
        """
        return dict([(k, encode(v))  for k, v in self.items()])

    def _get_strings(self):
        """
        Get a list of key=values strings as they appear in ParameterSet files.
        """
        return ["%s = %s" % (k, encode(v))  for k, v in self.items()]

    def __str__(self):
        return "\n".join(self._get_strings())

    def __repr__(self):
        return self.__str__()

    def to_file(self, filename):
        f = open(filename, 'w')
        f.write(str(self)+'\n')
        f.close()

    def __iter__(self):
        yield iter(self.keys())

    def items(self):
        return [(k, self.get_value(k)) for k in self.keys()]

def encode(value):
    """Encode a python value as ParameterSet string"""

    def single_str(value):
        if isinstance(value, bool):
            return value and 'true' or 'false'
        if isinstance(value, str):
            return value
        return value
    value = single_str(value)
    # deal with numpy arrays, by converting to lists
    if hasattr(value, 'tolist'):
        value = value.tolist()
    if isinstance(value, list) or isinstance(value, tuple):
        def to_str(value):
            if isinstance(value, list) or isinstance(value, tuple):
                for i in value:
                    return "[" + ", ".join([to_str(i) for i in value])  + "]"
            else:
                if isinstance(value, bool):
                    return value and 'true' or 'false'
                return str(value)

        if len(value) > 2 and not isinstance(value[0], list):
            # [ n * val ]
            if value == [value[0]]*len(value):
                val = str(single_str(value[0]))
                return '[' + str(len(value)) + ' * ' + val + ']'
            # n..m
            elif isinstance(value[0], int) \
                and value == range(value[0], value[-1]+1):
                return str(value[0]) + '..' + str(value[-1])
        return to_str(value)
    return str(value)

def decode(value):
    """
    This function takes text a string which is using ParameterSet syntax
    and is decoding it into a valid python value, e.g.:: python

        >>> p = ParameterSet('x.y', '[a,b]')
        >>> print decode(p.x.y)
        ['a', 'b']

    Supported value encodings are:

    * ranges i..j or j..i with padding, e.g. abc00..09.txt
    * lists
    * numerical arrays (lists of lists with numerical values)
    * booleans true/false

    """
    if not isinstance(value, basestring):
        return value
    rxislist = re.compile(r"^\[(.+)\]$")
    rxbool = re.compile(r"([tT]rue|[fF]alse)")
    rxisrange = re.compile(r"(\d+)\.{2}(\d+)")
    rxisnum = re.compile(r"^([+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?)$")
    # lists/arrays
    match = rxislist.match(value)
    if match:
        # check for [ n * <value> ] and expand
        # doesn't work for vectors elements
        if value.count(",")  == 0:
            rxtimes = re.compile(r"\[.*?(\d+)\s*[*](.+)\]")
            mtch = rxtimes.match(value)
            if mtch:
                fact = int(mtch.groups()[0])
                val = mtch.groups()[1].strip()
                if rxbool.match(val):
                    val = eval(val.title())
                elif rxisnum.match(val):
                    val = eval(val)
                return fact*[val]

        # dodgey way to test for arrays of numerical values
        # don't support any other array type
        if value.count("[") > 1:
            try:
                out = eval(value)
                return out
            except:
                raise ValueError("Can't decode arrays of non-numbers")
        out = []
        items = match.groups()[0].split(",")
        for i in items:
            i = i.strip()
            if rxisnum.match(i):
                i = eval(i)
            out.append(i)
        return out
    # look for  '01..10' type pattern
    match = rxisrange.search(value)
    if match:
        r = match.groups()
        r0 = int(r[0])
        r1 = int(r[1])
        sgn = (r0 < r1) and 1 or -1
        rng  = range(r0, r1+sgn, sgn)
        # just numerical range
        if match.span()[0] == 0 and match.span()[1] == len(value):
            return rng
        nwidth = max(len(r[0]), len(r[1]))
        strg = rxisrange.sub("%%0%ii" % nwidth, value)
        return [ strg % i for i in rng ]
    # int/float
    if rxisnum.match(value):
        return eval(value)
    # true/false
    if rxbool.match(value):
        return eval(value.title())
    return value

def extract(line):
    """
    Return a key/value pair from a string. This will most likely be a line in a
    ParameterSet file
    """
    line = line.strip()
    if len(line) == 0 or line.startswith("#"):
        return None
    kv = line.split("=",1)
    return kv[0].strip(),kv[1].strip()
