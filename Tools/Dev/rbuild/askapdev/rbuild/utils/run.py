## Package for various utility functions to execute build and shell commands
#
# @copyright (c) 2007 CSIRO
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
# @author Malte Marquarding <Malte.Marquarding@csiro.au>
#
import os
import sys

from ..exceptions import BuildError
from parse_error import parse_error

OPTIONS = sys.argv[1:]
OPTIONSTRING = ' '.join(OPTIONS)

def run(unixcomm, extraquiet=False, ignore_traceback=False):
    '''run a command through os.system, with optional suppression of stdout
    :param unixcomm: The string for the unix command
    :param extraquiet: suppress everything but errors

    .. warning::

    Deprecated - do not use in new code - use runcmd instead.
    '''
    err = "err.log"
    quiet = '-q' in OPTIONS

    if quiet:
        unixcomm += " > /dev/null"
        if extraquiet:
            unixcomm += " 2> %s" % err

    status = os.system(unixcomm)

    if status > 0 and not ignore_traceback:
        if quiet and extraquiet and os.path.exists(err):
            errormsg = parse_error(err)
            print >> sys.stderr, errormsg
            os.remove(err)
        raise BuildError(unixcomm)

    if quiet and extraquiet and os.path.exists(err):
        os.remove(err)
