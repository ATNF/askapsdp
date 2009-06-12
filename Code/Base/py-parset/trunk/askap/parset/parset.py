import os
import re

from askap.parset import logger

class ParameterSet(object):
    """Create a ParameterSet:
    The default constructor creates an empty  ParameterSet instance.

    :param args: if only one argument, this is assumed to be the file name of
                 a ParameterSet file. If two arguments are provided this is 
                 assumed to be a key and value.
    :param kw:   key/value parameters

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
        # from key, value
        elif len(args) == 2:
            self._add(*args)
        elif len(kw):
            for k,v in kw.iteritems():
                self._add(k, v)

    def _add(self, k, v):
        """Add a key/value pair to the object:
        
        :param k: the key string, e.g. 'x' or 'x.y'
        :param v: the value string, using ParameterSet syntax 
        """
        keys = k.split(".")
        k = keys[0]
        tail = None
        if len(keys) > 1:
            tail = ".".join(keys[1:])
        if k in self._keys:
            child = self.__dict__[k]
            if isinstance(child, ParameterSet):
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
        """Return a python dict object representation"""
        out = {}
        for k in self._keys:
            if isinstance(self.__dict__[k], ParameterSet):
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
            if isinstance(self.__dict__[k], ParameterSet):
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
    """Decode a ParameterSet value into python types
    """
    if not isinstance(value, str):
        return value
    rxislist = re.compile(r"^\[(.+)\]$")
    rxbool = re.compile(r"([tT]rue|[fF]alse)")
    rxisrange = re.compile(r"(\d+)\.{2}(\d+)")
    rxisnum = re.compile(r"^((\A|(?<=\W))(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?)$")
    # lists/arrays
    match = rxislist.match(value)
    if match:
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
    """Extract a key/value pair form a string"""
    line = line.strip()
    if len(line) == 0 or line.startswith("#"):
        return None
    kv = line.split("=")
    return kv[0].strip(),kv[1].strip()
