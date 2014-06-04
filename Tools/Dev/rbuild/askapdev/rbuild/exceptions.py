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
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

__all__ = ["BuildError"]

import sys

class BuildError(Exception):
    """An exception to be raise for any known build process error.
    This is treated with a special :func:`sys.excepthook` as to not
    print any traceback on fail just the message prefixed by 'error: '.
    """
    pass

def except_hook(etype, value, tb):
    if isinstance(value, BuildError):
        print >>sys.stderr, "error:", value
    else:
        sys.__excepthook__(etype, value, tb)
    sys.exit(1)

sys.excepthook = except_hook
