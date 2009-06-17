.. askap.parset documentation master file, created by sphinx-quickstart on Thu Mar 19 11:46:41 2009.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. module:: askap.parset
   :synopsis: Create a pythonic representation of LOFAR ParameterSets

.. moduleauthor:: Malte Marquarding <Malte.Marquarding@csiro.au>
.. sectionauthor:: Malte Marquarding <Malte.Marquarding@csiro.au>

:mod:`askap.parset` --- LOFAR ParameterSet parser
=================================================

ParameterSets are usually defined as text files for configuration purposes.
Here is an example::

    Cimager.Images.Names                             = image00..12.tenuJy_simtest
    Cimager.Images.image.i.tenuJy_simtest.shape      = [2048,2048]
    Cimager.Images.image.i.tenuJy_simtest.cellsize   = [8.0arcsec, 8.0arcsec]
    Cimager.Images.image.i.tenuJy_simtest.frequency  = [1.420e9,1.420e9]
    Cimager.Images.image.i.tenuJy_simtest.nchan      = 1
    Cimager.Images.image.i.tenuJy_simtest.direction  = [12h30m00.00, -45.00.00.00, J2000]


ParameterSet values are always strings, as they are mainly stored in files.
This modules offers the :func:`decode` function to help parsing these into python
types.

:class:`ParameterSet` is a little bit more restrictive compared to the
original ParameterSet class:

    * only leaf nodes can have values
    * nested list vectors (lists) can only contain numerical values
    * lists can't contain expressions
    * keys should be valid as variable names, e.g. the following wouldn't work
      '10uJy' or 'abc-xyz'

.. class:: ParameterSet(*args, **kw)

    The default constructor creates an empty  ParameterSet instance.
    ParameterSet keys can be accessed as attributes::

        p = ParameterSet('x.y.z', 1)
        print p.x.y.z
        >>> 1

    or dictionary keys::

        p = ParameterSet('x.y.z', 1)
        print p["x"]["y"]["z"]
        >>> 1

    All methods are private (prefixed "_"), so that only ParameterSet values
    show up as public attributes.


    :param args: if only one argument, this is assumed to be the file name of
                 a ParameterSet file. If two arguments are provided this is
                 assumed to be a key and a value.
    :param kw:   key/value parameters

    Example::

        p0 = ParameterSet()
        p1 = ParameterSet('x.y', 1)
        p2 = ParameterSet(x=1, y=2, z=ParameterSet('a', 3))
        print x.a
        >>> 3
        p3 = ParameterSet('xyz.parset')
        p1._add('x.a', 2)


.. method:: ParameterSet._add

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

.. method:: ParameterSet._to_dict()

    Returns a python :class:`dict` representation of the `ParameterSet`, decoding
    all values

.. function:: decode(value)

    This function takes text a string which is using ParameterSet syntax
    and is decoding it into a valid python value, e.g.::

        p = ParameterSet('x.y', '[a,b]')
        print decode(p.x.y)
        >>> ['a', 'b']

.. function:: extract(line)

    return a key/value pair from a string. This will most likely be a line in a
    ParameterSet file

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
