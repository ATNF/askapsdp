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
    from askap import logging
except ImportError:
    import logging

# module attributes
logger = logging.getLogger(__name__)

from askap.parset.parset import ParameterSet, decode, encode, extract, to_dict
