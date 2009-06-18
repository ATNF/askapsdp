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

    All methods are private (prefixed "_"), so that only ParameterSet values
    show up as public attributes.


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
        >>> p3 = ParameterSet('xyz.parset')
        >>> p1._add('x.a', 2)

    """
    def __init__(self, *args, **kw):
        object.__setattr__(self, "_keys", [])
        # from file
        if len(args) == 1:
            if isinstance(args[0], str) and os.path.exists(args[0]):
                pfile = file(args[0], "r")
                i = 1
                for line in pfile:
                    pair = extract(line)
                    if pair:
                        try:
                            self._add(*pair)
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
            self._add(*args)
        elif len(kw):
            for k,v in kw.iteritems():
                self._add(k, v)
        elif len(args) == 0 and len(kw) == 0:
            pass
        else:
            raise ValueError("Incorrect arguments to constructor.")

    def _add(self, k, v):
        """
        Add a key/value pair. This will recursively create keys if necessart
        when the key contains '.' notation. This is the only way to add keys of
        this form. To set non-nested attributes one can use attribute or
        item set notation, so that the following are equivalent::
            
            p = ParameterSet()
            p.x = 1
            p["x"] = 1
            p._add("x", 1)
            # to add nested keys use
            p._add('x.y', 1)
            # this fails
            p.x.y = 1
            # as does this
            p["x"]["y"]

        :param k: key
        :param v: value

        """
        keys = k.split(".")
        k = keys[0]
        tail = None
        if len(keys) > 1:
            tail = ".".join(keys[1:])
        if k in self._keys:
            child = self.__dict__[k]
            if isinstance(child, self.__class__):
                child._add(tail, v)
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
                child._add(tail, v)
                self.__dict__[k] = child
            self._keys.append(k)

    def __setitem__(self, k, v):
        self._add(k, v)

    def __setattr__(self, k, v):
        self._add(k, v)

    def __getitem__(self, k):
        if k in self._keys:
            return decode(self.__dict__[k])
        else:
            raise KeyError

    def _to_dict(self):
        """
        Returns a python :class:`dict` representation of the `ParameterSet`,
        decoding all values using :func:`decode`
        """
        out = {}
        for k in self._keys:
            if isinstance(self.__dict__[k], self.__class__):
                out[k] = self.__dict__[k]._to_dict()
            else:
                out[k] = decode(self.__dict__[k])
        return out

    def _get_strings(self):
        """
        Get a list of key=values strings as they appear in ParameterSet files.
        """
        out = []
        for k in self._keys:
            if isinstance(self.__dict__[k], self.__class__):
                children = self.__dict__[k]._get_strings()
                for child in children:
                    out.append(".".join([k, child]))
            else:
                out.append("%s = " % k + str(self.__dict__[k]))
        return out

    def __str__(self):
        return "\n".join(self._get_strings())

    def __repr__(self):
        return self.__str__()

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
    if not isinstance(value, str):
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
                if rxisnum.match(val):
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
    kv = line.split("=")
    return kv[0].strip(),kv[1].strip()