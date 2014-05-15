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

import os.path

def parse_error(errfile, ncontext=2):
    '''
    Trim the output from error/warning messages from an error file
    with a number of lines of context.
    :param errfile: error filename
    :param ncontext: number of lines of context around errors and warnings.
    '''
    if not os.path.exists(errfile):
        return None

    f = file(errfile, "r")
    lines = f.readlines()
    f.close()
    outlines = []
    context = 0

    for line in lines[::-1]:
        low = line.lower()
        if low.find("error") > -1 and low.find("warning") == -1:
            outlines.append(line)
            context = ncontext
        elif context > 0:
            outlines.append(line)
            context -= 1
            if context == 0:
                outlines.append("<snip>...\n")
        else:
            pass

    return ' '.join(outlines[::-1])
