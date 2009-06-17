"""
ParameterSets are usually defined as text files for configuration purposes.
Here is an example::

    cimager.images.names                             = image00..12.tenuJy_simtest
    cimager.images.image.i.tenuJy_simtest.shape      = [2048,2048]
    cimager.images.image.i.tenuJy_simtest.cellsize   = [8.0arcsec, 8.0arcsec]
    cimager.images.image.i.tenuJy_simtest.frequency  = [1.420e9,1.420e9]
    cimager.images.image.i.tenuJy_simtest.nchan      = 1
    cimager.images.image.i.tenuJy_simtest.direction  = [12h30m00.00, -45.00.00.00, J2000]


ParameterSet values are always strings as they are mainly stored in files.
This modules offers the :func:`decode` function to help parsing these into python
types and the :func:`extract` to extract a key/value pair form a string.

:class:`ParameterSet` is a little bit more restrictive compared to the
original ParameterSet class:

    * only leaf nodes can have values
    * nested list vectors (lists) can only contain numerical values
    * lists can't contain expressions
    * keys should be valid as variable names, e.g. the following wouldn't work
      '10uJy' or 'abc-xyz'

This module provides logging through :attr:`askap.parset.logger`.

"""
# if used outside askapsoft
try:
    import initenv
except ImportError:
    pass
# if used outside askapsoft
try:
    from askap import logging
except ImportError:
    import logging

# module attributes
logger = logging.getLogger(__name__)

from askap.parset.parset import ParameterSet, decode, extract
